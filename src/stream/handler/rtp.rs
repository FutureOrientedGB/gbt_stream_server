use std::net::SocketAddr;

use super::StreamHandler;

use rtp;

use webrtc_util::Unmarshal;


impl StreamHandler {
    pub fn on_rtp(&self, addr: SocketAddr, buff: &[u8]) -> bool {
        let mut b = &buff[..];
        match rtp::packet::Packet::unmarshal(&mut b) {
            Err(e) => {
                tracing::error!(
                    "rtp::packet::Packet::unmarshal error, e: {:?}",
                    e
                );
                return false;
            }
            Ok(rtp_packet) => {
                return true;
            }
        }
    }
}