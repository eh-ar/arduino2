#include <SPI.h>
#include <Ethernet.h>

// Network configuration (adjust for your network)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Replace with your MAC
IPAddress ip(192, 168, 1, 17);                        // Arduino's IP (or use DHCP)
IPAddress gateway(192, 168, 1, 1);                    // Your router's IP
IPAddress subnet(255, 255, 255, 0);                   // Your subnet mask

EthernetClient client;

void setup() {
  Serial.begin(9600);

  Serial.println("Initializing Ethernet...");
  // Use DHCP:
  // if (Ethernet.begin(mac) == 0) {
  // Or use static IP:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Ethernet shield initialization failed");

    while (true)
      ;  // Halt
  } else {
    Ethernet.begin(mac, ip, gateway, subnet);
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  }

  delay(1000);  // Small delay after initialization
}


void loop() {
  if (client.connect("www.google.com", 80)) {
    Serial.println("Connected to Google");
    client.stop();
  } else {
    Serial.println("Connection failed");
  }
  delay(10000);
}

void loop2() {
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("\nStarting ping tests...");

  // Ping Google DNS (8.8.8.8)
  ping("8.8.8.8");
  ping("192.168.1.179");
  // Ping Cloudflare DNS (1.1.1.1)
  ping("1.1.1.1");

  // Ping time.nist.gov (you can also use the IP directly)
  ping("time.nist.gov");  // Or ping(129, 6, 15, 28);

  delay(10000);  // Wait 10 seconds before next round of pings
}

void ping(const char* host) {
  Serial.print("Pinging ");
  Serial.print(host);
  Serial.print("...");

  if (client.connect(host, 80)) {  // Use port 80 for the ping test
    Serial.println("Success!");
    Serial.println(client.status());
    client.stop();
  } else {
    Serial.println("Failed!");
    Serial.println(client.status());
  }
}



// Overload the ping function to accept IPAddress as well
void ping2(IPAddress ip) {
  Serial.print("Pinging ");
  Serial.print(ip);
  Serial.print("...");

  if (client.connect(ip, 80)) {  // Use port 80 for the ping test
    Serial.println("Success!");
    client.stop();
  } else {
    Serial.println("Failed!");
    Serial.println(client.status());
  }
}
