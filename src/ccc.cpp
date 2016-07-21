/*****************************************************************************
Copyright (c) 2001 - 2011, The Board of Trustees of the University of Illinois.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the
  above copyright notice, this list of conditions
  and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the University of Illinois
  nor the names of its contributors may be used to
  endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu, last updated 02/21/2013
*****************************************************************************/


#include "core.h"
#include "ccc.h"
#include <cmath>
#include <cstring>
#include <bits/stdc++.h>

using namespace std;

double intial_cw = 16.0;

CCC::CCC():
m_iSYNInterval(CUDT::m_iSYNInterval),
m_dPktSndPeriod(1.0),
m_dCWndSize(intial_cw),
m_iBandwidth(),
m_dMaxCWndSize(),
cw_threshold(),
m_iMSS(),
m_iSndCurrSeqNo(),
m_iRcvRate(),
m_iRTT(),
m_pcParam(NULL),
m_iPSize(0),
m_dMinRTT(0),
m_ssThresh(100),
m_epochStart(0),
m_wLastMax(0),
m_k(0),
m_c(0.4),
m_beta(0.2),
m_originPoint(0),
m_cWndCnt(0),
m_UDT(),
m_iACKPeriod(0),
m_iACKInterval(0),
m_bUserDefinedRTO(false),
m_iRTO(-1),
m_PerfInfo()
{
}

CCC::~CCC()
{
   delete [] m_pcParam;
}

double CCC::getCW()
{
   return m_dCWndSize;
}


void CCC::setACKTimer(int msINT)
{
   m_iACKPeriod = msINT > m_iSYNInterval ? m_iSYNInterval : msINT;
}

void CCC::setACKInterval(int pktINT)
{
   m_iACKInterval = pktINT;
}

void CCC::setRTO(int usRTO)
{
   m_bUserDefinedRTO = true;
   m_iRTO = usRTO;
}

void CCC::sendCustomMsg(CPacket& pkt) const
{
   CUDT* u = CUDT::getUDTHandle(m_UDT);

   if (NULL != u)
   {
      pkt.m_iID = u->m_PeerID;
      u->m_pSndQueue->sendto(u->m_pPeerAddr, pkt);
   }
}

const CPerfMon* CCC::getPerfInfo()
{
   try
   {
      CUDT* u = CUDT::getUDTHandle(m_UDT);
      if (NULL != u)
         u->sample(&m_PerfInfo, false);
   }
   catch (...)
   {
      return NULL;
   }

   return &m_PerfInfo;
}

void CCC::setMSS(int mss)
{
   m_iMSS = mss;
}

void CCC::setBandwidth(int bw)
{
   m_iBandwidth = bw;
}

void CCC::setSndCurrSeqNo(int32_t seqno)
{
   m_iSndCurrSeqNo = seqno;
}

void CCC::setRcvRate(int rcvrate)
{
   m_iRcvRate = rcvrate;
}

void CCC::setMaxCWndSize(int cwnd)
{
   m_dMaxCWndSize = cwnd;
}

void CCC::setCWThreshold(int threshold)
{
   cw_threshold = threshold;
}


void CCC::setRTT(int rtt)
{
   m_iRTT = rtt;
}

void CCC::setUserParam(const char* param, int size)
{
   delete [] m_pcParam;
   m_pcParam = new char[size];
   memcpy(m_pcParam, param, size);
   m_iPSize = size;
}

//
CUDTCC::CUDTCC():
m_iRCInterval(),
m_LastRCTime(),
m_bSlowStart(),
m_iLastAck(),
m_bLoss(),
m_iLastDecSeq(),
m_dLastDecPeriod(),
m_iNAKCount(),
m_iDecRandom(),
m_iAvgNAKNum(),
m_iDecCount()
{
}

void CUDTCC::init()
{
   m_iRCInterval = m_iSYNInterval;
   m_LastRCTime = CTimer::getTime();
   // setACKTimer(m_iRCInterval);
   setACKInterval(8);
   setRTO(m_iSYNInterval);
   // setCWThreshold(m_dMaxCWndSize*0.75);
   m_bSlowStart = true;
   m_iLastAck = m_iSndCurrSeqNo;
   // m_bLoss = false;
   m_iLastDecSeq = CSeqNo::decseq(m_iLastAck);
   m_dLastDecPeriod = 1;
   m_iAvgNAKNum = 0;
   m_iNAKCount = 0;
   m_iDecRandom = 1;
   
   m_dMinRTT = 0;
   m_ssThresh = 100;
   m_dCWndSize = intial_cw;
   m_epochStart = 0;
   m_wLastMax = 0;
   m_c = 0.4;
   m_beta = 0.2;
   m_originPoint = 0;
   m_cWndCnt = 0;
   m_dPktSndPeriod = 1;
}

void CUDTCC::onACK(int32_t ack)
{
    // cout<<"On Ack cWnd ------------------" <<m_dCWndSize<<endl;
   uint64_t currtime = CTimer::getTime();
   if (currtime - m_LastRCTime < (uint64_t)m_iRCInterval){
      return;
   }

   m_LastRCTime = currtime;

   if(m_dMinRTT == 0) {
      m_dMinRTT = m_iRTT;
   }
   else{
      m_dMinRTT = std::min(m_dMinRTT, m_iRTT);
   }

   // Slow start mode, add one segSize to cWnd. Default m_ssThresh is 65535. (RFC2001, sec.1)
   if(m_dCWndSize <= m_ssThresh){
      m_dCWndSize += CSeqNo::seqlen(m_iLastAck, ack);
      m_iLastAck = ack;
   }
   else{
       // Congestion avoidance mode
      if (CubicUpdate () < m_cWndCnt){
         m_dCWndSize += CSeqNo::seqlen(m_iLastAck, ack);
         m_cWndCnt = 0;
      }
      else{
         m_cWndCnt++;
         m_iLastAck = ack;

      }
   }

   // if (m_dCWndSize > m_dMaxCWndSize)
   //    {
   //       m_bSlowStart = false;
   //       m_dCWndSize = m_dMaxCWndSize - 1;
   // }
}


void CUDTCC::onLoss(const int32_t* losslist, int)
{
   m_epochStart = 0;
   if(m_dCWndSize < m_wLastMax){
      m_wLastMax = 0.9*m_dCWndSize;
   }
   else{
      m_wLastMax = m_dCWndSize;
   }
   m_dCWndSize = 0.8*m_dCWndSize > 1 ? 0.8*m_dCWndSize: 1;
   // cout<<"Loss, new Congestion CW : "<<m_dCWndSize<<endl;

}

void CUDTCC::onTimeout()
{
   // m_bSlowStart = true;
   // m_dCWndSize = m_dCWndSize*0.75 > 1 ? m_dCWndSize*0.75: 1;
   CubicReset ();
   m_ssThresh = m_ssThresh/2;
   m_dCWndSize = intial_cw;
   // cout<<"Timed out in CCC new CW = "<<m_dCWndSize<<endl;


}


double CUDTCC::CubicUpdate (void){
  /*
   * Window increment by cubic function
   */  
  uint32_t cWnd = m_dCWndSize;
    
  if (m_epochStart <= 0)
   {

     m_epochStart = (double)CTimer::getTime();
     
     if (cWnd < m_wLastMax)
      {
        m_k = pow((m_wLastMax - cWnd) / m_c, (1.0 / 3.0));
        m_originPoint = m_wLastMax;
      }
     else
      {
        m_k = 0.0;
        m_originPoint = cWnd;
      }
   }
   
  double t = (double)CTimer::getTime() + m_dMinRTT - m_epochStart;
  double target = m_originPoint + m_c * pow(t - m_k, 3.0);
  
  double cnt;
   
  if (target > cWnd)
   {
     cnt = cWnd / (target - cWnd);
   }
  else
   {
     cnt = 100u * cWnd;
   }
  
  return cnt;
}


void CUDTCC::CubicReset (void){
  /*
   * Asdf
   */
  m_wLastMax = 0;
  m_epochStart = (double)CTimer::getTime();
  m_originPoint = 0;
  m_dMinRTT = 0;
  m_k = 0.0;
  m_cWndCnt = 0;
}


