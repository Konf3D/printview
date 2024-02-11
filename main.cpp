#include <iostream>
#include <vector>
#include <xlnt/xlnt.hpp>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

// Function to retrieve page count for a printer given its IP address
int getPageCount(const std::string& ipAddress) {
    snmp_session session, *ss;
    snmp_pdu *pdu, *response;
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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.xlsx>" << std::endl;
        return 1;
    }

    std::vector<std::string> ipAddresses;
    std::vector<int> pageCounts;

    // Open Excel file
    xlnt::workbook wb;
    try {
        wb.load(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error opening Excel file: " << e.what() << std::endl;
        return 1;
    }

    // Read IP addresses from the first row
    auto ws = wb.active_sheet();
    for (auto cell : ws.rows().front()) {
        if (cell.to_string().empty()) {
            break;
        }
        ipAddresses.push_back(cell.to_string());
    }

    // Retrieve page counts for each printer
    for (const auto& ipAddress : ipAddresses) {
        int pageCount = getPageCount(ipAddress);
        pageCounts.push_back(pageCount);
        std::cout << "Page count for printer at " << ipAddress << ": " << pageCount << std::endl;
    }

    // Write page counts to the last empty row
    auto lastRow = ws.cell(ws.highest_column() + 1, 1);
    for (size_t i = 0; i < pageCounts.size(); ++i) {
        lastRow.offset(i, 0).value(pageCounts[i]);
    }

    wb.save(argv[1]);

    return 0;
}
