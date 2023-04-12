// nptTime.cpp, mostly from chatGPT v3


#include "theClock3.h"
#include <myIOTLog.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <myIOTdevice.h>


static unsigned int localPort = 2390;      // local port to listen for UDP packets

// modified, theirs was 3600,3600
// so with DST turned on, this gave 7200.
// NOTE that this IS NOT what is displayed in the log time,
// because THAT goes through the time zone settings in myIOTHttp.cpp NTP code ...
// AND I *think* that passing a GMT time to the web browser shows it in the machine's timezone,
// AND NOTE that display of times with the command line is NOT working.


static const long  gmtOffset_sec = 0;		// time(NULL) returns GMT?
static const int   daylightOffset_sec = 0;		// was 3600;

// my additions

static const int NTP_PACKET_SIZE = 48;
static byte packetBuffer[ NTP_PACKET_SIZE];

static WiFiUDP  udp;
IPAddress timeServer;
static bool udp_started = 0;



static void sendNTPpacket(IPAddress &address)
	// send an NTP request to the time server at the given address
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:

	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}



bool getNtpTime(int32_t *secs, int32_t *ms)
{
	if (WiFi.status() != WL_CONNECTED)
		return 0;
	if (udp_started == -1)
		return 0;

	if (!udp_started)
	{
		udp_started = -1;
		LOGD("starting UDP");

		#if WITH_NTP	// myIOT version
			String ntpServer = my_iot_device->getString(ID_NTP_SERVER);
		#else
			String ntpServer = "pool.ntp.org";
		#endif

		WiFi.hostByName(ntpServer.c_str(), timeServer);
		LOGD("%s = %s",ntpServer.c_str(),timeServer.toString().c_str());
		if (!udp.begin(localPort))
		{
			LOGE("Could not start UDP on localPort(%d)",localPort);
			return 0;
		}
		LOGD("UDP started on localPort(%d)",localPort);
		udp_started = 1;
	}

	while (udp.parsePacket() > 0) ; // discard any previously received packets

	LOGD("Transmit NTP Request");
	sendNTPpacket(timeServer);

	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500)
	{
		int size = udp.parsePacket();
		if (size >= NTP_PACKET_SIZE)
		{
			// LOGD("Receive NTP Response");
			udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer

			// convert four bytes starting at location 40 to a long integer
			uint32_t usecs =  (unsigned long)packetBuffer[40] << 24;
			usecs |= (unsigned long)packetBuffer[41] << 16;
			usecs |= (unsigned long)packetBuffer[42] << 8;
			usecs |= (unsigned long)packetBuffer[43];
			usecs -= 2208988800UL + gmtOffset_sec + daylightOffset_sec;
			*secs = usecs;

			#define NTP_MAX_INT_AS_DOUBLE  (4294967295.0)  // Max value of frac

			uint32_t int_fraction =(unsigned long)packetBuffer[44] << 24;
			int_fraction |= (unsigned long)packetBuffer[45] << 16;
			int_fraction |= (unsigned long)packetBuffer[46] << 8;
			int_fraction |= (unsigned long)packetBuffer[47];
			float f_fraction = int_fraction;
			f_fraction /= NTP_MAX_INT_AS_DOUBLE;
			float f_millis = f_fraction * 1000L;
			*ms = f_millis;

			LOGU("NTP_RESPONSE secs=%d  i_frac=0x%08x f_frac=%03f f_millis=%03f ms=%03d",
				 *secs,
				 int_fraction,
				 f_fraction,
				 f_millis,
				 *ms);
			return secs;
		}
	}

	LOGE("No NTP Response :-(");
	return 0; // return 0 if unable to get the time
}



void syncNTPTime()
	// This snippet, copied from myIOT::myIOTHTTP.cpp, causes the
	// ESP32 to synchronize it's clock to the given NTP server.
	// It is an obtruse way of doing things, and I have no idea
	// ho long it takes.
{
        LOGD("Synchronizing to ntpServer");
        const char* ntpServer = "pool.ntp.org";
        uint32_t tz_enum = my_iot_device->getEnum(ID_DEVICE_TZ);
        const char *tz_string = tzString(static_cast<IOT_TIMEZONE>(tz_enum));
        LOGI(" using TZ(%d)=%s",tz_enum,tz_string);
        configTime(0, 0, ntpServer);	// <<-- THE magic is here
        setenv("TZ", tz_string, 1);
        tzset();

}
