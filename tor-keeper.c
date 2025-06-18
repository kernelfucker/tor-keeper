/* See LICENSE file for license details */
/* tor-keeper - system-wide tor keeper/tunnel */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define tor "/usr/bin/tor"
#define torrc "/etc/tor/torrc"
#define iptables "/usr/bin/iptables"
#define ip6tables "/usr/bin/ip6tables"
#define pidf "/var/run/torctrl.pid"
#define tor_pidf "/var/run/tor.pid"
#define logf "/var/log/torctrl.log"

typedef struct{
	int strictnodes;
	int random_exitnodes;
	char exitnodes[256];
	pid_t tor_pid;
	time_t st_time;
	volatile sig_atomic_t interrupted;
} torstate;

torstate ts = {0, 0, "", 0, 0, 0};

void logm(const char *m){
	FILE *f = fopen(logf, "a");
	if(f){
		time_t now = time(NULL);
		fprintf(f, "[%.24s] %s\n", ctime(&now), m);
		fclose(f);
	}

	printf("%s\n", m);
}

int is_tor_running(){
	FILE *f = popen("pgrep -x tor", "r");
	if(!f) return 0;
	char buf[16];
	int running = (fgets(buf, sizeof(buf), f) != NULL);
	pclose(f);
	return running;
}

void kill_tor(){
	system("killall tor");
	sleep(1);
	if(is_tor_running()){
		system("killall -9 tor");
		sleep(1);
	}
}

void configure_t_proxy(){
	logm("[+] configuring firewall rules");

	system(iptables " -F");
	system(iptables " -t nat -F");
	system(iptables " -t mangle -F");
	system(ip6tables " -F");
	system(ip6tables " -t nat -F");
	system(ip6tables " -t mangle -F");

	system(iptables " -P INPUT ACCEPT");
	system(iptables " -P FORWARD DROP");
	system(iptables " -P OUTPUT ACCEPT");
	system(ip6tables " -P INPUT ACCEPT");
	system(ip6tables " -P FORWARD DROP");
	system(ip6tables " -P OUTPUT ACCEPT");  

	system(iptables " -A INPUT -i lo -j ACCEPT");
	system(iptables " -A OUTPUT -o lo -j ACCEPT");
	system(ip6tables " -A INPUT -i lo -j ACCEPT");
	system(ip6tables " -A OUTPUT -o lo -j ACCEPT");

	system(iptables " -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT");
	system(iptables " -A OUTPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT");
	system(iptables " -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT");
	system(ip6tables " -A OUTPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT"); 

	system(iptables " -A OUTPUT -m owner --uid-owner root -j ACCEPT");
	system(ip6tables " -A OUTPUT -m owner --uid-owner root -j ACCEPT");  

	system(iptables " -t nat -A OUTPUT -p tcp -m owner ! --uid-owner root -j REDIRECT --to-ports 9040");
	system(ip6tables " -t nat -A OUTPUT -p tcp -m owner ! --uid-owner root -j REDIRECT --to-ports 9040");

	system(iptables " -t nat -A OUTPUT -p udp --dport 53 -j REDIRECT --to-ports 5353");
	system(ip6tables " -t nat -A OUTPUT -p udp --dport 53 -j REDIRECT --to-ports 5353");

	logm("[+] firewall configured for system-wide tor keeper/tunnel");
}

void st_tor(){
	logm("[+] starting tor");
	if(is_tor_running()){
		logm("[!] tor is already running, what the hell are you trying to do?");
		return;
	}

	system("mkdir --parents /etc/tor/");
	FILE *rc = fopen(torrc, "w");
	if(!rc){
		logm("[!] cannot creating torrc");
		exit(1);
	}

	fprintf(rc, "SocksPort 9050\n");
	fprintf(rc, "RunAsDaemon 1\n");
	fprintf(rc, "Sandbox 1\n");
	fprintf(rc, "PidFile " tor_pidf "\n");
	fprintf(rc, "User root\n");
	fprintf(rc, "TransPort 9040\n");
	fprintf(rc, "DNSPort 5353\n");
	fprintf(rc, "AutomapHostsOnResolve 1\n");
	fprintf(rc, "Log notice file /var/log/tor/notice.log\n");
	if(ts.random_exitnodes){
		fprintf(rc, "ExitNodes {??}\n");
	} else if(strlen(ts.exitnodes) > 0 ){
		fprintf(rc, "ExitNodes %s\n", ts.exitnodes);
	}

	if(ts.strictnodes){
		fprintf(rc, "StrictNodes 1\n");
	}

	fclose(rc);

	system("mkdir --parents /var/lib/tor/");
	system("chown -R root:root /var/lib/tor/");

	int r = system(tor " -f " torrc);
	if(r != 0){
		logm("[!] failed to start tor");
		exit(1);
	}

	ts.st_time = time(NULL);
	logm("[+] tor started, waiting 15 seconds to initialize..");
	sleep(15);
	configure_t_proxy();
	if(is_tor_running()){
		logm("[+] tor is running");
		logm("[+] system-wide tor keeper/tunnel is active");
		logm("[!] verify with: $ netstat -tlnpau | grep -F 9050");
	} else {
		logm("[!] tor failed to start");
		logm("[!] check /var/log/tor/notices.log for log details");
	}
}

void stop_tor(){
	logm("[+] stopping tor");

	system(iptables " -F");
	system(iptables " -t nat -F");
	system(iptables " -t mangle -F");
	system(ip6tables " -F");
	system(ip6tables " -t nat -F");
	system(ip6tables " -t mangle -F");

	system(iptables " -P INPUT ACCEPT");
	system(iptables " -P FORWARD ACCEPT");
	system(iptables " -P OUTPUT ACCEPT");
	system(ip6tables " -P INPUT ACCEPT");
	system(ip6tables " -P FORWARD ACCEPT");
	system(ip6tables " -P OUTPUT ACCEPT");

	kill_tor();
	logm("[+] tor stopped and firewall has been restored to normal");
}

void force_stop_tor(){
	logm("[+] force stopping tor");
	kill_tor();
	logm("[+] tor stopped");
}

void status(){
	if(is_tor_running()){
		printf("[+] tor is running\n");
		system("echo [+] hostname: $(uname -n)\n");
		system("echo [+] kernel: $(uname -r)\n");	
		system("echo [+] user: $(whoami)\n");
		printf("[!] verify with: $ netstat -tlnpau | grep -F 9050\n");
	} else {
		printf("[!] tor is not running, check that port 9050 is open\n");
	}
}

void help(){
	printf("tor-keeper - system-wide tor keeper/tunnel\n");
	printf("note: this program must be run as root\n");
	printf("usage: tor-keeper [options]..\n");
	printf("options:\n");
	printf("  --start		start tor with transparent proxy on keeper\n");
	printf("  --stop		stop tor and reset firewall\n");
	printf("  --force-stop		force kill tor without resetting firewall\n");
	printf("  --status		show current tor status and few information\n");
	printf("  --strict		enable strict nodes selection\n");
	printf("  --random		use random exit nodes\n");
	printf("  --exit-nodes X	set custom exit nodes, comma-separated\n");
	printf("  --help		display this\n");
}

int main(int argc, char *argv[]){
	if(argc < 2){
		help();
		return 0;
	}

	if(getuid() != 0){
		printf("tor-keeper: this program must be run as root\n");
		return 1;
	}

	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "--start") == 0){
			st_tor();
		}
		else if(strcmp(argv[i], "--stop") == 0){
			stop_tor();
		}
		else if(strcmp(argv[i], "--force-stop") == 0){
			force_stop_tor();
		}
		else if(strcmp(argv[i], "--status") == 0){
			status();
		}
		else if(strcmp(argv[i], "--strict") == 0){
			ts.strictnodes = 1;
		}
		else if(strcmp(argv[i], "--random") == 0){
			ts.random_exitnodes = 1;
		}
		else if(strcmp(argv[i], "--exit-nodes") == 0 && i+1 < argc){
			strncpy(ts.exitnodes, argv[++i], sizeof(ts.exitnodes)-1);
		}
		else if(strcmp(argv[i], "--help") == 0){
			help();
		}
	}

	return 0;
}
