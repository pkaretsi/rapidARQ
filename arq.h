#include "connector.h"
#include "ranvar.h"
#include <set>

class ARQTx;
enum ARQStatus {IDLE,SENT,ACKED,RTX,DROP}; //statuses for packets sent by ARQTx
enum PacketStatus {NONE,MISSING,RECEIVED,DECODED}; //for ARQAcker, in order to tell apart different types of packets

class ARQHandler : public Handler {
public:
    ARQHandler(ARQTx& arq) : arq_tx_(arq) {};
    void handle(Event*);
private:
    ARQTx& arq_tx_;
};

class ACKEvent : public Event {
 public:
	int ACK_sn;
	int ACK_uid;
	Packet* coded_ack;
	bool isCancelled;
};

class ARQTx : public Connector {
 public:
	ARQTx();
	void recv(Packet*, Handler*);
	void nack(int rcv_sn, int rcv_uid);
	void ack(int rcv_sn, int rcv_uid);
	void ack(Packet *p); //overloaded ack method
	void resume();
	int command(int argc, const char*const* argv);
	//functions for setting protocol parameters
	int get_ratek() {return rate_k;}
	int get_codingdepth() {return coding_depth;}
	double get_linkdelay() {return lnk_delay_;}
	double get_linkbw() {return lnk_bw_;}
	int get_apppktsize() {return app_pkt_Size_;}
	//functions used in statistics logging
	double get_start_time() {return start_time;}
	int get_total_packets_sent() {return packets_sent;}
  int get_total_retransmissions() {return pkt_rtxs;}
	double get_pkt_tx_start(int seq_num) {return pkt_tx_start[seq_num%wnd_];}
 protected:
	ARQHandler arqh_;
	Handler* handler_;

	Packet *pending; //used for storing a packet from queue that finds the channel blocked_

	int wnd_;  //window size
	int sn_cnt; //the total count of used sequence numbers
	int retry_limit_; //maximum number of retransmissions allowed for each frame

	Packet **pkt_buf; //buffer used for storing frames under transmission (maximum size of wnd_)
	ARQStatus *status; //the status of each frame under transmission
	int *num_rtxs; //number of retransmisions for each frame under transmission
	int *pkt_uids; //used for debugging purposes

	int blocked_; //switch set to 1 when Tx engaged in transmiting a frame, 0 otherwise
	int last_acked_sq_; //sequence number of last acked frame
	int most_recent_sq_; //sequence number of most recent frame to be sent
	int num_pending_retrans_; //number of frames needed to be retransmitted (after the first attempt)
	int rate_k; //number of native packets sent before coded
	int coding_depth; //the number of coding cycles used to create a coded

	double lnk_bw_; //the bandwidth of the link_
	double lnk_delay_; //the delay of the link_
	int app_pkt_Size_; //the size of the pkt created by the app_pkt_Size_

	bool debug;

	//Statistics
	double start_time; //time when 1st packet arrived at ARQTx::recv
	int packets_sent; //unique packets sent
  int pkt_rtxs; //the total number of retransmissions
	double *pkt_tx_start; //the start time of a packet's transmission

	int findpos_retrans();
	void reset_lastacked();
	Packet* create_coded_packet();

 private:
	int native_counter; //counts the number of native packets sent before sending a coded one

};

class ARQRx : public Connector {
 public:
	ARQRx();
	//ARQRx()  {arq_tx_=0; };
	int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*) = 0;
 protected:
	ARQTx* arq_tx_;

	double delay_; //delay for returning feedback
	double timeout_; //the time used to trigger nack()
};

class ARQNacker;

class ARQAcker : public ARQRx {
public:
	ARQAcker();
	virtual void handle(Event*);
	void recv(Packet*, Handler*);
	int command(int argc, const char*const* argv);
	void print_stats();
	void log_lost_pkt(Packet *p, ACKEvent *e);
	void delete_nack_event(int seq_num);
 protected:
	int wnd_;  //window size
	int sn_cnt; //the total count of used sequence numbers
	Packet **pkt_buf; //buffer used for storing packets arriving out of order
	Packet **lost_pkt_buf; //buffer used for storing lost packets
	ACKEvent **event_buf; //buffer for storing pointers to NACK events
	int last_fwd_sn_; //sequence number of the last frame forwarded to the upper layer
	int most_recent_acked; //sequence number of the last frame for which an ack has been sent

	RandomVariable *ranvar_; //a random variable for generating errors in ACK delivery
	double err_rate; //the rate of errors in ACK delivery

	PacketStatus *status; //status of received packets
	set<int> known_packets; //already correctly received packets
	set<int> lost_packets; //how many packets are lost
	int received_coded_cnt; //counter for received coded packets that are available for decoding

	ARQNacker* nacker;

	bool debug;

	//Statistics
	double finish_time; //time when the last pkt was delivered to the receiver's upper layer, used to calculate throughput
	int delivered_pkts; //the total number of pkts delivered to the receiver's upper layer
	int delivered_data; //the total number of bytes delivered to the receiver's upper layer
	double sum_of_delay; //sum of delays for every packet delivered, used to calculate average delay
	int avg_dec_matrix_size; //the avg size of the decoding matrix when decoding is performed (used to estimate processing overhead)
	int num_of_decodings; //number of decoding operations

	void deliver_frames(int steps, bool mindgaps, Handler *h);
	void clean_decoding_matrix(int from, int to);

 private:
	Packet* create_coded_ack();
	void parse_coded_packet(Packet *p, Handler *h);
	void parse_coded_ack(Packet *p);
	void decode(Handler* h, bool afterCoded);

};

class ARQNacker : public ARQRx {
 public:
	ARQNacker();
	virtual void handle(Event*);
	void recv(Packet*, Handler*);
	int command(int argc, const char*const* argv);
 protected:
	ARQAcker* acker;
	bool debug;
};
