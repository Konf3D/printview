#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

// Function to retrieve page count for a printer given its IP address
int getPageCount(const std::string& ipAddress) {
    struct snmp_session session, *ss;
    struct snmp_pdu *pdu, *response;
    oid anOID[] = {1, 3, 6, 1, 2, 1, 43, 10, 2, 1, 4, 1, 1}; // Example OID for page count

    snmp_sess_init(&session);
    session.peername = strdup(ipAddress.c_str());
    session.version = SNMP_VERSION_2c;
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);
    ss = snmp_open(&session);

    if (!ss) {
        snmp_perror("snmp_open");
        return -1; // Failed attempt
    }

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    snmp_add_null_var(pdu, anOID, OID_LENGTH(anOID));

    int status = snmp_synch_response(ss, pdu, &response);
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
        for (netsnmp_variable_list *vars = response->variables; vars; vars = vars->next_variable) {
            if (vars->type == ASN_COUNTER) {
                return *vars->val.integer;
            }
        }
    }

    return -1; // Failed attempt
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt> <output_file.txt>" << std::endl;
        return 1;
    }

    std::vector<std::string> ipAddresses;
    std::vector<int> pageCounts;

    // Open input file
    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        return 1;
    }

    // Read IP addresses from input file
    std::string ipAddress;
    while (std::getline(input, ipAddress)) {
        ipAddresses.push_back(ipAddress);
    }
    input.close();

    // Retrieve page counts for each printer
    for (const auto& ipAddress : ipAddresses) {
        int pageCount = getPageCount(ipAddress);
        pageCounts.push_back(pageCount);
        std::cout << "Page count for printer at " << ipAddress << ": " << pageCount << std::endl;
    }

    // Write page counts to output file with current date in the filename
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::stringstream filename;
    filename << std::put_time(&tm, "%Y-%m-%d") << ".txt";
    std::ofstream output(filename.str());
    if (!output.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return 1;
    }

    for (const auto& pageCount : pageCounts) {
        output << pageCount << std::endl;
    }
    output.close();

    return 0;
}
