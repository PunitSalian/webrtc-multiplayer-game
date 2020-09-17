#ifndef NETWORK_H
#define NETWORK_H

#include <api/create_peerconnection_factory.h>
#include <api/data_channel_interface.h>
#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <websocket.h>
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
  bool SetRemoteDescription(const char *type, const char *sdp);
  bool AddIceCandidate(const char *sdp, const int sdp_mlineindex,
                       const char *sdp_mid);

  // make it abstract as this might cause diamond hierachy
  virtual void AddRef() const = 0;
  virtual rtc::RefCountReleaseStatus Release() const = 0;

protected:
  // Callback for when the WebSocket server receives a message from the client.
  // static void OnWebSocketMessage(websocket_incoming_message msg);

  // create a peerconneciton and add the turn servers info to the configuration.
  bool CreatePeerConnection(const char **turn_urls, const int no_of_urls,
                            const char *username, const char *credential);
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
  // static Websocket ws_;
  // disallow copy-and-assign
  WebrtcNetwork(const WebrtcNetwork &) = delete;
  WebrtcNetwork &operator=(const WebrtcNetwork &) = delete;
};

namespace GameNetwork {

bool NetworkInit();

bool Connect(std::vector<std::string> &);

void NewConnection(std::string &name, std::string &offer);

void Senddata(std::string &name, std::string &data);

void Disconnect(std::vector<std::string> &);

} // namespace GameNetwork

#endif
