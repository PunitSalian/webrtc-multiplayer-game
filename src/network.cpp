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

class Wrapper : public webrtc::PeerConnectionObserver,
                public webrtc::CreateSessionDescriptionObserver,
                public webrtc::SetSessionDescriptionObserver,
                public webrtc::DataChannelObserver {


Wrapper(){}
~Wrapper(){}

void on_message(const std::string &) {}

// webrtc::PeerConnectionObserver
// Override signaling change.
void OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState new_state) override {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::SignalingChange(" << new_state << ")"
            << std::endl;
}

// Override data channel change.
void OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {
  std::cout << ":" << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::DataChannel(" << data_channel << ", "
            << data_channel.get() << ")" << std::endl;
}

// Override renegotiation.
void OnRenegotiationNeeded() {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::RenegotiationNeeded" << std::endl;
}

// Override ICE connection change.
void OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState /* new_state */)override {}

// Override ICE gathering change.
void OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState new_state) {

  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceGatheringChange(" << new_state << ")"
            << std::endl;
}

// Override ICE candidate.
void OnIceCandidate(
    const webrtc::IceCandidateInterface *candidate) override {
  std::cout << std::this_thread::get_id() << ":"
            << "PeerConnectionObserver::IceCandidate" << std::endl;
}

// DataChannel events.
// class DataChannelObserver : public webrtc::DataChannelObserver {

// Change in state of the Data Channel.
void OnStateChange() {}

// Message received.
void OnMessage(const webrtc::DataBuffer &buffer) override{
  std::string(buffer.data.data<char>(), buffer.data.size());
}

// Buffered amount change.
void OnBufferedAmountChange(uint64_t /* previous_amount */) {}

// Create SessionDescription events.
// : public webrtc::CreateSessionDescriptionObserver {

// Successfully created a session description.
void OnSuccess(webrtc::SessionDescriptionInterface *desc) override {
  peer_connection_->SetLocalDescription(this, desc);
}

// Failure to create a session description.
void OnFailure(webrtc::RTCError error) override {
  std::cout << name << ":" << std::this_thread::get_id() << ":"
            << "CreateSessionDescriptionObserver::OnFailure" << std::endl
            << error.message() << std::endl;
}

// Set SessionDescription events.
//  : public webrtc::SetSessionDescriptionObserver {

// Successfully set a session description.
void OnSuccess() override {
  std::cout << name << ":" << std::this_thread::get_id() << ":"
            << "SetSessionDescriptionObserver::OnSuccess" << std::endl;
}
private:

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
  std::string name;

};
/*
class Wrapper {
public:
  Wrapper(const std::string name_,Websocket* ws) : name(name_),ws_(ws),
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

  Wrapper w("webrtc conection");

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
