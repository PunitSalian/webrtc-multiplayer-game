#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <api/create_peerconnection_factory.h>
#include <network.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/strings/json.h>
#include <rtc_base/thread.h>
#include <system_wrappers/include/field_trial.h>
#include <websocket.h>

struct Ice {
  std::string candidate;
  std::string sdp_mid;
  int sdp_mline_index;
};


WebrtcNetwork::WebrtcNetwork(){}
WebrtcNetwork::~WebrtcNetwork(){}

bool WebrtcNetwork::InitializePeerConnection(){

}

void WebrtcNetwork::DeletePeerConnection(){

}


bool WebrtcNetwork::CreateDataChannel(){

}


bool WebrtcNetwork::CreateOffer(){

}


bool WebrtcNetwork::CreateAnswer(){

}

bool WebrtcNetwork::SendDataViaDataChannel(const std::string& data){


}


// PeerConnectionObserver implementation.
void WebrtcNetwork::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState new_state)  {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::SignalingChange(" << new_state << ")"
            << std::endl;
}

// Override OnAddStream
void WebrtcNetwork::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)  {
      std::cout << name_ << ":" << std::this_thread::get_id() << ":"
                << "PeerConnectionObserver::AddStream" << std::endl;
}

// Override OnRemoveStream
void WebrtcNetwork::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)  {
      std::cout << name_ << ":" << std::this_thread::get_id() << ":"
                << "PeerConnectionObserver::RemoveStream" << std::endl;
}

// Override data channel change.
void WebrtcNetwork::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)  {
  std::cout << ":" << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::DataChannel(" << data_channel << ", "
            << data_channel.get() << ")" << std::endl;
}

// Override renegotiation.
void WebrtcNetwork::OnRenegotiationNeeded() {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::RenegotiationNeeded" << std::endl;
}

// Override ICE connection change.
void WebrtcNetwork::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState /* new_state */) {}

// Override ICE gathering change.
void WebrtcNetwork::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)  {

  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceGatheringChange(" << new_state << ")"
            << std::endl;
}

// Override ICE candidate.
void WebrtcNetwork::OnIceCandidate(
    const webrtc::IceCandidateInterface *candidate)  {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceCandidate" << std::endl;
}

void WebrtcNetwork::OnIceConnectionReceivingChange(bool receiving){

}

// CreateSessionDescriptionObserver implementation
// Successfully created a session description.
void WebrtcNetwork::OnSuccess(webrtc::SessionDescriptionInterface *desc)  {
  peer_connection_->SetLocalDescription(this, desc);
}

// Failure to create a session description.
void WebrtcNetwork::OnFailure(webrtc::RTCError error)  {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "CreateSessionDescriptionObserver::OnFailure" << std::endl
            << error.message() << std::endl;
}

// SetSessionDescriptionObserver implementation
// Successfully set a session description.
void WebrtcNetwork::OnSuccess()  {
  std::cout << name_ << ":" << std::this_thread::get_id() << ":"
            << "SetSessionDescriptionObserver::OnSuccess" << std::endl;
}


// DataChannelObserver implementation.
void WebrtcNetwork::OnStateChange(){
      std::cout << name_ << ":" << std::this_thread::get_id() << ":"
                      << "DataChannelObserver::StateChange" << std::endl;
}

void WebrtcNetwork::OnMessage(const webrtc::DataBuffer& buffer){
std::cout << name_ << ":" << std::this_thread::get_id() << ":"
                << "DataChannelObserver::Message" << std::endl;
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
