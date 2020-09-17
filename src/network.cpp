#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <thread>

#include <network.h>
#include <rtc_base/strings/json.h>
namespace {
struct Ice {
  std::string candidate;
  std::string sdp_mid;
  int sdp_mline_index;
};

static std::unordered_map<std::string, rtc::scoped_refptr<WebrtcNetwork>>
    connections;

static Websocket ws_;

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

  if (type == "offer") {
    auto offerdata = root["data"].asString();
    auto name = root["name"].asString();
    GameNetwork::NewConnection(name, offerdata);
  } else if (type == "candidate") {

  } else if (type == "candidate") {
  }
}

} // namespace

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    WebrtcNetwork::peer_connection_factory = nullptr;

WebrtcNetwork::WebrtcNetwork(std::string name) : name_(name) {}
WebrtcNetwork::~WebrtcNetwork() {
  network_thread->Stop();
  worker_thread->Stop();
  signaling_thread->Stop();
}

bool WebrtcNetwork::InitializePeerConnection() {

  std::cout << ":" << std::this_thread::get_id() << ":"
            << "init Main thread" << std::endl;

  // Using Google's STUN server.
  if (peer_connection_factory == nullptr) {
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
    peer_connection_factory =
        webrtc::CreateModularPeerConnectionFactory(std::move(dependencies));

    if (peer_connection_factory.get() == nullptr) {
      std::cout << ":" << std::this_thread::get_id() << ":"
                << "Error on CreateModularPeerConnectionFactory." << std::endl;
      exit(EXIT_FAILURE);
    }
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
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
  }
  webrtc::SdpParseError error;
  webrtc::SessionDescriptionInterface *session_description(
      webrtc::CreateSessionDescription("offer", parameter, &error));
  if (session_description == nullptr) {
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Error on CreateSessionDescription." << std::endl
              << error.line << std::endl
              << error.description << std::endl;
    std::cout << name_ << ":" << std::this_thread::get_id() << ":"
              << "Offer SDP:begin" << std::endl
              << parameter << std::endl
              << "Offer SDP:end" << std::endl;
    exit(EXIT_FAILURE);
  }
  peer_connection_->SetRemoteDescription(this, session_description);
  peer_connection_->CreateAnswer(
      this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
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
  ws_.send(sdp);
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

bool GameNetwork::NetworkInit() {
  ws_.setreceivehandler(OnWebSocketMessage);

  if (ws_.connect(U("localhost:9090")) == FAILURE) {
    return false;
  }

  return WebrtcNetwork::InitializePeerConnection();
}

bool GameNetwork::Connect(std::vector<std::string> &peers) {
  for (auto &name : peers) {
    connections[name] = new rtc::RefCountedObject<WebrtcNetwork>(name);
    connections[name]->CreateOffer();
  }
}

void GameNetwork::NewConnection(std::string &name, std::string &offer) {
  connections[name] = new rtc::RefCountedObject<WebrtcNetwork>(name);
  connections[name]->CreateAnswer(offer);
}
void GameNetwork::Senddata(std::string &name, std::string &data) {
  connections[name]->SendDataViaDataChannel(data);
}

void GameNetwork::Disconnect(std::vector<std::string> &peers) {

  for (auto &name : peers) {

    if (connections.count(name) > 0) {
      connections[name]->DeletePeerConnection();
    }
  }
}

/*
class WebrtcNetwork {
public:
  WebrtcNetwork(const std::string name_,Websocket* ws) : name(name_),ws_(ws),
connection(name_) {}

  void init() {
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "init Main thread" << std::endl;

    webrtc::PeerConnectionInterface::IceServer ice_server;
    ice_server.uri = "stun:stun.l.google.com:19302";

    rtc::InitializeSSL();
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
    peer_connection_factory =
        webrtc::CreateModularPeerConnectionFactory(std::move(dependencies));
    if (peer_connection_factory.get() == nullptr) {
      std::cout << name << ":" << std::this_thread::get_id() << ":"
                << "Error on CreateModularPeerConnectionFactory." << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  void create_offer_sdp() {
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "create_offer_sdp" << std::endl;

    connection.peer_connection = peer_connection_factory->CreatePeerConnection(
        configuration, nullptr, nullptr, &connection.pco);

    webrtc::DataChannelInit config;

    // Configuring DataChannel.
    connection.data_channel =
        connection.peer_connection->CreateDataChannel("data_channel", &config);
    connection.data_channel->RegisterObserver(&connection.dco);

    if (connection.peer_connection.get() == nullptr) {
      peer_connection_factory = nullptr;
      std::cout << name << ":" << std::this_thread::get_id() << ":"
                << "Error on CreatePeerConnection." << std::endl;
      exit(EXIT_FAILURE);
    }
    connection.peer_connection->CreateOffer(
        connection.csdo,
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  }

  void push_reply_sdp(const std::string &parameter) {
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "push_reply_sdp" << std::endl;

    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface *session_description(
        webrtc::CreateSessionDescription("answer", parameter, &error));
    if (session_description == nullptr) {
      std::cout << name << ":" << std::this_thread::get_id() << ":"
                << "Error on CreateSessionDescription." << std::endl
                << error.line << std::endl
                << error.description << std::endl;
      std::cout << name << ":" << std::this_thread::get_id() << ":"
                << "Answer SDP:begin" << std::endl
                << parameter << std::endl
                << "Answer SDP:end" << std::endl;
      exit(EXIT_FAILURE);
    }
    connection.peer_connection->SetRemoteDescription(connection.ssdo,
                                                     session_description);
  }

  void push_ice(const Ice &ice_it) {
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "push_ice" << std::endl;

    webrtc::SdpParseError err_sdp;
    webrtc::IceCandidateInterface *ice = CreateIceCandidate(
        ice_it.sdp_mid, ice_it.sdp_mline_index, ice_it.candidate, &err_sdp);
    if (!err_sdp.line.empty() && !err_sdp.description.empty()) {
      std::cout << name << ":" << std::this_thread::get_id() << ":"
                << "Error on CreateIceCandidate" << std::endl
                << err_sdp.line << std::endl
                << err_sdp.description << std::endl;
      exit(EXIT_FAILURE);
    }
    connection.peer_connection->AddIceCandidate(ice);
  }

  static void receive_websocket_message(websocket_incoming_message msg) {}

  void send(const std::string &parameter) {
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "send" << std::endl;

    webrtc::DataBuffer buffer(
        rtc::CopyOnWriteBuffer(parameter.c_str(), parameter.size()), true);
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "Send(" << connection.data_channel->state() << ")"
              << std::endl;
    connection.data_channel->Send(buffer);
  }

  void quit() {
    rtc::CleanupSSL();
    std::cout << name << ":" << std::this_thread::get_id() << ":"
              << "quit" << std::endl;
    connection.peer_connection->Close();
    connection.peer_connection = nullptr;
    connection.data_channel = nullptr;
    peer_connection_factory = nullptr;

    network_thread->Stop();
    worker_thread->Stop();
    signaling_thread->Stop();
  }

private:
  const std::string name;
  Websocket* ws_;
  std::unique_ptr<rtc::Thread> network_thread;
  std::unique_ptr<rtc::Thread> worker_thread;
  std::unique_ptr<rtc::Thread> signaling_thread;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory;
  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  Connection connection;
};

void Game_network::connect(std::string player1name, std::string player2name) {
  bs = new Websocket();
  bool isconnect = bs->connect(U("localhost:9090"));

  if (!isconnect) {
    std::cout << "Not connected to signalling server" << std::endl;
    return;
  }

  // login
  {
    Json::Value login;
    login["type"] = "login";
    login["name"] = player1name;
    Json::StyledWriter writer;
    std::string strJson = writer.write(login);
    std::cout << "JSON WriteTest" << std::endl << strJson << std::endl;
    bs->send(strJson);
  }

  WebrtcNetwork w("webrtc conection");

  w.init();

  w.create_offer_sdp();

  // offer for connection
  {
    Json::Value offer;

    offer["type"] = "offer";

    offer["name"] = player2name;

    //offer["data"] = data;

    Json::StyledWriter writer;

    std::string strJson = writer.write(offer);

    std::cout << "JSON WriteTest" << std::endl << strJson << std::endl;

    bs->send(strJson);
  }
}

*/
