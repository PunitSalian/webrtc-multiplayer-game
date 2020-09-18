#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <thread>

#include <network.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/strings/json.h>
#include <rtc_base/thread.h>
//namespace {
struct Ice {
  std::string candidate;
  std::string sdp_mid;
  int sdp_mline_index;
};

class WebrtcNetwork : public webrtc::PeerConnectionObserver,
                      public webrtc::CreateSessionDescriptionObserver,
                      public webrtc::SetSessionDescriptionObserver,
                      public webrtc::DataChannelObserver {
public:
  WebrtcNetwork(std::string);
  ~WebrtcNetwork();
  static bool InitializePeerConnection();
  void DeletePeerConnection();
  bool CreateOffer();
  bool CreateAnswer(const std::string &parameter);
  bool SendDataViaDataChannel(const std::string &data);

  // Register callback functions.
  bool SetRemoteDescription(std::string &);
  bool AddIceCandidate(std::string &sdp, int sdp_mlineindex,
                       std::string &sdp_mid);

  // make it abstract as this might cause diamond hierachy
  virtual void AddRef() const = 0;
  virtual rtc::RefCountReleaseStatus Release() const = 0;

protected:
  void CloseDataChannel();
  // PeerConnectionObserver implementation.
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override;
  void
  OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

  void OnRemoveStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
  void OnRenegotiationNeeded() override;
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
  void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override;
  void OnIceConnectionReceivingChange(bool receiving) override;

  // CreateSessionDescriptionObserver implementation.
  void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
  void OnFailure(webrtc::RTCError error) override;

  // SetSessionDescriptionObserver implementation.
  void OnSuccess() override;

  // DataChannelObserver implementation.
  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer &buffer) override;

private:
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
  std::string name_;
  static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory;
  static std::unique_ptr<rtc::Thread> network_thread;
  static std::unique_ptr<rtc::Thread> worker_thread;
  static std::unique_ptr<rtc::Thread> signaling_thread;
  static webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  // disallow copy-and-assign
  WebrtcNetwork(const WebrtcNetwork &) = delete;
  WebrtcNetwork &operator=(const WebrtcNetwork &) = delete;
};

static std::unordered_map<std::string, rtc::scoped_refptr<WebrtcNetwork>>
    connections;

static Websocket ws_;

// Wrapper around messages recieved from websocket
void NewConnectionmessage(std::string &name, std::string &offerdata) {
  std::cout << "offer from = " << name << std::endl;
  connections[name] = new rtc::RefCountedObject<WebrtcNetwork>(name);
  connections[name]->CreateAnswer(offerdata);
}

void Addcandidatemessage(std::string &name, std::string &candidatedata) {
  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(candidatedata.c_str(), root);

  if (!parsingSuccessful) {
    std::cout << "Bad parsing 1" << std::endl;
    return;
  }

  std::cout << candidatedata << std::endl;

  auto sdp = root["candidate"].asString();

  auto sdp_mid = root["sdpMid"].asString();

  auto sdp_mlineindex = root["sdpMLineIndex"].asInt();

  connections[name]->AddIceCandidate(sdp, sdp_mlineindex, sdp_mid);
}

void AnswerRecieved(std::string &name, std::string &answerdata) {
  connections[name]->SetRemoteDescription(answerdata);
}

void OnWebSocketMessage(websocket_incoming_message msg) {

  auto strJson = msg.extract_string().get();
  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(strJson.c_str(), root); // parse process

  if (!parsingSuccessful) {
    std::cout << "Bad parsing " << std::endl;
    return;
  }
  std::cout << strJson << std::endl;

  auto type = root["type"].asString();
  auto name = root["name"].asString();

  if (type == "offer") {
    auto offerdata = root["data"].asString();
    NewConnectionmessage(name, offerdata);
  } else if (type == "candidate") {
    auto candidates = root["candidate"].asString();
    Addcandidatemessage(name, candidates);
  } else if (type == "answer") {
    auto answerdata = root["answer"].asString();
    AnswerRecieved(name, answerdata);
  }
}

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    WebrtcNetwork::peer_connection_factory = nullptr;
std::unique_ptr<rtc::Thread>  WebrtcNetwork::network_thread;
std::unique_ptr<rtc::Thread>  WebrtcNetwork::worker_thread;
std::unique_ptr<rtc::Thread>  WebrtcNetwork::signaling_thread;
webrtc::PeerConnectionInterface::RTCConfiguration  WebrtcNetwork::configuration;


  // disallow copy-and-assign
WebrtcNetwork::WebrtcNetwork(std::string name) : name_(name) {}
WebrtcNetwork::~WebrtcNetwork() {
  WebrtcNetwork::network_thread->Stop();
  WebrtcNetwork::worker_thread->Stop();
  WebrtcNetwork::signaling_thread->Stop();
  rtc::CleanupSSL();
}

bool WebrtcNetwork::InitializePeerConnection() {

  std::cout << ":" << std::this_thread::get_id() << ":"
            << "init Main thread" << std::endl;

  // Using Google's STUN server.
  if (peer_connection_factory == nullptr) {
    rtc::InitializeSSL();
    webrtc::PeerConnectionInterface::IceServer ice_server;
    ice_server.uri = "stun:stun.l.google.com:19302";
    configuration.servers.push_back(ice_server);

    network_thread = rtc::Thread::CreateWithSocketServer();
    network_thread->Start();
    worker_thread = rtc::Thread::Create();
    worker_thread->Start();
    signaling_thread = rtc::Thread::Create();
    signaling_thread->Start();
    webrtc::PeerConnectionFactoryDependencies dependencies;
    dependencies.network_thread = network_thread.get();
    dependencies.worker_thread = worker_thread.get();
    dependencies.signaling_thread = signaling_thread.get();
    WebrtcNetwork::peer_connection_factory =
        webrtc::CreateModularPeerConnectionFactory(std::move(dependencies));

    if (WebrtcNetwork::peer_connection_factory.get() == nullptr) {
      std::cout << ":" << std::this_thread::get_id() << ":"
                << "Error on CreateModularPeerConnectionFactory." << std::endl;
      return false;
    }
    std::cout << "PeerConnectionfactory created with the config" << std::endl;
  }
}

void WebrtcNetwork::DeletePeerConnection() {
  CloseDataChannel();
  peer_connection_->Close();
  peer_connection_ = nullptr;
}

bool WebrtcNetwork::CreateOffer() {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "create_offer_sdp" << std::endl;

  peer_connection_ = peer_connection_factory->CreatePeerConnection(
      configuration, nullptr, nullptr, this);

  webrtc::DataChannelInit config;

  // Configuring DataChannel.
  data_channel_ = peer_connection_->CreateDataChannel("data_channel", &config);
  data_channel_->RegisterObserver(this);

  if (peer_connection_.get() == nullptr) {
    peer_connection_factory = nullptr;
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Error on CreatePeerConnection." << std::endl;
    return false;
  }
  peer_connection_->CreateOffer(
      this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}

bool WebrtcNetwork::CreateAnswer(const std::string &parameter) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "create_answer_sdp" << std::endl;

  peer_connection_ = peer_connection_factory->CreatePeerConnection(
      configuration, nullptr, nullptr, this);

  if (peer_connection_.get() == nullptr) {
    peer_connection_factory = nullptr;
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Error on CreatePeerConnection." << std::endl;

    return false;
  }
  webrtc::SdpParseError error;
  std::unique_ptr<webrtc::SessionDescriptionInterface> session_description(
      webrtc::CreateSessionDescription("offer", parameter, &error));
  if (!session_description.get()) {
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Error on CreateSessionDescription." << std::endl
              << error.line << std::endl
              << error.description << std::endl;
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Offer SDP:begin" << std::endl
              << parameter << std::endl
              << "Offer SDP:end" << std::endl;
    return false;
  }
  peer_connection_->SetRemoteDescription(this, session_description.get());
  peer_connection_->CreateAnswer(
      this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

  return true;
}

bool WebrtcNetwork::SendDataViaDataChannel(const std::string &data) {
  if (!data_channel_.get()) {
    std::cout << "Data channel is not established" << std::endl;
    return false;
  }
  webrtc::DataBuffer buffer(data);
  data_channel_->Send(buffer);
  return true;
}

bool WebrtcNetwork::SetRemoteDescription(std::string &parameter) {

  webrtc::SdpParseError error;
  std::unique_ptr<webrtc::SessionDescriptionInterface> session_description(
      webrtc::CreateSessionDescription("anwer", parameter, &error));

  if (!session_description.get()) {
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Error on CreateSessionDescription." << std::endl
              << error.line << std::endl
              << error.description << std::endl;

    return false;
  }
  peer_connection_->SetRemoteDescription(this, session_description.get());

  return true;
}

bool WebrtcNetwork::AddIceCandidate(std::string &sdp_mid, int sdp_mline_index,
                                    std::string &candidate) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "AddIceCandidate" << std::endl;

  webrtc::SdpParseError err_sdp;
  std::unique_ptr<webrtc::IceCandidateInterface> ice(webrtc::CreateIceCandidate(
      sdp_mid, sdp_mline_index, candidate, &err_sdp));
  if (!err_sdp.line.empty() && !err_sdp.description.empty() && !ice.get()) {
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Error on CreateIceCandidate" << std::endl
              << err_sdp.line << std::endl
              << err_sdp.description << std::endl;
    return false;
  }
  peer_connection_->AddIceCandidate(ice.get());
}

void WebrtcNetwork::CloseDataChannel() {
  if (data_channel_.get()) {
    data_channel_->UnregisterObserver();
    data_channel_->Close();
  }
  data_channel_ = nullptr;
}

// PeerConnectionObserver implementation.
void WebrtcNetwork::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState new_state) {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::SignalingChange(" << new_state << ")"
            << std::endl;
}

// Override OnAddStream
void WebrtcNetwork::OnAddStream(
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::AddStream" << std::endl;
}

// Override OnRemoveStream
void WebrtcNetwork::OnRemoveStream(
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::RemoveStream" << std::endl;
}

// Override data channel change.
void WebrtcNetwork::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  std::cout << ":" << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::DataChannel(" << data_channel << ", "
            << data_channel.get() << ")" << std::endl;

  data_channel_ = data_channel;

  data_channel_->RegisterObserver(this);
}

// Override renegotiation.
void WebrtcNetwork::OnRenegotiationNeeded() {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::RenegotiationNeeded" << std::endl;
}

// Override ICE connection change.
void WebrtcNetwork::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceConnectionChange(" << new_state
            << ")" << std::endl;
}

// Override ICE gathering change.
void WebrtcNetwork::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState new_state) {

  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceGatheringChange(" << new_state << ")"
            << std::endl;
}

// Override ICE candidate.
void WebrtcNetwork::OnIceCandidate(
    const webrtc::IceCandidateInterface *candidate) {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceCandidate" << std::endl;
  Ice ice;
  candidate->ToString(&ice.candidate);
  ice.sdp_mid = candidate->sdp_mid();
  ice.sdp_mline_index = candidate->sdp_mline_index();

  Json::Value icedata;
  Json::Value candidatedata;
  icedata["type"] = "candidate";
  icedata["name"] = name_;

  candidatedata["candidate"] = ice.candidate;

  candidatedata["sdpMid"] = ice.sdp_mid;

  candidatedata["sdpMLineIndex"] = ice.sdp_mline_index;

  icedata["candidate"] = candidatedata;
  Json::StyledWriter writer;
  std::string strJson = writer.write(icedata);
  std::cout << "JSON icecandidate" << std::endl << strJson << std::endl;
  ws_.send(strJson);
}

void WebrtcNetwork::OnIceConnectionReceivingChange(bool receiving) {}

// CreateSessionDescriptionObserver implementation
// Successfully created a session description.
void WebrtcNetwork::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
  peer_connection_->SetLocalDescription(this, desc);
  std::string sdp;
  desc->ToString(&sdp);
  std::cout << " sdp:begin" << std::endl << sdp << " sdp:end" << std::endl;
  Json::Value sessiondata;
  Json::StyledWriter writer;
  std::string sessiontype =
      (desc->GetType() == webrtc::SdpType::kOffer) ? "offer" : "answer";
  sessiondata["type"] = sessiontype;
  sessiondata["name"] = name_;
  sessiondata[sessiontype] = sdp;
  std::string strJson = writer.write(sessiondata);
  std::cout << "JSON WriteTest" << std::endl << strJson << std::endl;
  ws_.send(strJson);
}

// Failure to create a session description.
void WebrtcNetwork::OnFailure(webrtc::RTCError error) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "CreateSessionDescriptionObserver::OnFailure" << std::endl
            << error.message() << std::endl;
}

// SetSessionDescriptionObserver implementation
// Successfully set a session description.

void WebrtcNetwork::OnSuccess() {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "SetSessionDescriptionObserver::OnSuccess" << std::endl;
}

// DataChannelObserver implementation.
void WebrtcNetwork::OnStateChange() {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "DataChannelObserver::StateChange" << std::endl;
}

void WebrtcNetwork::OnMessage(const webrtc::DataBuffer &buffer) {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "DataChannelObserver::Message" << std::endl;
}
//} // namespace
bool GameNetwork::NetworkInit(std::string &player1name) {
  ws_.setreceivehandler(OnWebSocketMessage);

  if (ws_.connect(U("localhost:9090")) == FAILURE) {
    std::cout << "Failure to connect to server" << std::endl;
    return false;
  }

  // login
  {
    Json::Value login;
    login["type"] = "login";
    login["name"] = player1name;
    Json::StyledWriter writer;
    std::string strJson = writer.write(login);
    std::cout << "JSON WriteTest" << std::endl << strJson << std::endl;
    ws_.send(strJson);
  }

  return WebrtcNetwork::InitializePeerConnection();
}

bool GameNetwork::Connect(std::vector<std::string> &peers) {
  for (auto &name : peers) {
    connections[name] = new rtc::RefCountedObject<WebrtcNetwork>(name);
    connections[name]->CreateOffer();
  }
}

void GameNetwork::Senddata(std::string &name, std::string &data) {
  if (connections.count(name) > 0) {
    connections[name]->SendDataViaDataChannel(data);
  }
}

void GameNetwork::Disconnect(std::vector<std::string> &peers) {

  for (auto &name : peers) {

    if (connections.count(name) > 0) {
      connections[name]->DeletePeerConnection();
    }
  }
}
