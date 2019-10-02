#ifndef __TIZENRT_RECV_H_
#define __TIZENRT_RECV_H_

extern sint _rtw_init_recv_priv(struct recv_priv *precvpriv, _adapter *padapter);
extern void _rtw_free_recv_priv(struct recv_priv *precvpriv);

extern s32 rtw_recv_entry(union recv_frame *precv_frame);
extern int rtw_recv_indicatepkt(_adapter *adapter, union recv_frame *precv_frame);
extern void rtw_recv_returnpacket(IN _nic_hdl cnxt, IN _pkt *preturnedpkt);

extern void rtw_hostapd_mlme_rx(_adapter *padapter, union recv_frame *precv_frame);
#if BAD_MIC_COUNTERMEASURE
extern void rtw_handle_tkip_mic_err(_adapter *padapter, u8 bgroup);
#endif

int rtw_init_recv_priv(struct recv_priv *precvpriv, _adapter *padapter);
void rtw_free_recv_priv(struct recv_priv *precvpriv);

int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, u8 *pdata, _pkt *pskb);
int rtw_os_recv_resource_init(struct recv_priv *precvpriv, _adapter *padapter);
int rtw_os_recv_resource_alloc(_adapter *padapter, union recv_frame *precvframe);
void rtw_os_recv_resource_free(struct recv_priv *precvpriv);

int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf);
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf);

void rtw_os_read_port(_adapter *padapter, struct recv_buf *precvbuf);

void rtw_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl);

void rltk_netif_rx(struct sk_buff *skb);

#endif //__TIZENRT_RECV_H_
