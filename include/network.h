#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>

#include <vector>
#include <string>
#include <map>
#include <memory>

#include <api/create_peerconnection_factory.h>

#include <api/data_channel_interface.h>
#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>


class WebrtcNetwork :public webrtc::PeerConnectionObserver,
                     public webrtc::CreateSessionDescriptionObserver,
                     public webrtc::SetSessionDescriptionObserver,
                     public webrtc::DataChannelObserver{
 public:
  WebrtcNetwork();
  ~WebrtcNetwork();
  bool InitializePeerConnection();
  void DeletePeerConnection();
  bool CreateDataChannel();
  bool CreateOffer();
  bool CreateAnswer();
  bool SendDataViaDataChannel(const std::string& data);
  
  
  // Register callback functions.
  bool SetRemoteDescription(const char* type, const char* sdp);
  bool AddIceCandidate(const char* sdp,
                       const int sdp_mlineindex,
                       const char* sdp_mid);
 protected:
  // create a peerconneciton and add the turn servers info to the configuration.
  bool CreatePeerConnection(const char** turn_urls,
                            const int no_of_urls,
                            const char* username,
                            const char* credential);
  void CloseDataChannel();
  // PeerConnectionObserver implementation.
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override;
  void OnAddStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
  void OnRemoveStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
  void OnRenegotiationNeeded() override;
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnIceConnectionReceivingChange(bool receiving) override;

  // CreateSessionDescriptionObserver implementation.
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
  void OnFailure(webrtc::RTCError error) override;

  // SetSessionDescriptionObserver implementation.
  void OnSuccess() override;

  // DataChannelObserver implementation.
  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;
 private:
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
  std::string name_;
  // disallow copy-and-assign
  WebrtcNetwork(const WebrtcNetwork&) = delete;
  WebrtcNetwork& operator=(const WebrtcNetwork&) = delete;
};


class GameNetwork{

    public:

    void NetworkInit(std::string&);
    
    bool Connect(std::vector<std::string>);
    
    void Senddata(std::string& data);

    void Disconnect(std::vector<std::string>);

    private:
    
    int peercount;
};

#endif
