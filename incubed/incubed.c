#include "incubed.h"
#include "verifier/in3_init.h"
#include "core/client/client.h"



///  using the basic-module to get and verify a Block with the API and also as direct RPC-call

#include "client.h"   // the core client
#include "eth_api.h"  // functions for direct api-access
#include "in3_init.h" // if included the verifier will automaticly be initialized.
#include "log.h"      // logging functions

#include <inttypes.h>
#include <stdio.h>

static void get_block_rpc(in3_t* in3);
static void get_block_api(in3_t* in3);

void testGetBlock() {
  // create new incubed client
    in3_t* in3 = in3_for_chain(ETH_CHAIN_ID_MAINNET);
    

  // get block using raw RPC call
    get_block_rpc(in3);

  // get block using API
    get_block_api(in3);

  // cleanup client after usage
    in3_free(in3);
}

void get_block_rpc(in3_t* in3) {
    // prepare 2 pointers for the result.
    char *result, *error;

    // send raw rpc-request, which is then verified
    in3_ret_t res = in3_client_rpc(
      in3,                    //  the configured client
      "eth_getBlockByNumber", // the rpc-method you want to call.
      "[\"latest\",true]",    // the arguments as json-string
      &result,                // the reference to a pointer whill hold the result
      &error);                // the pointer which may hold a error message

    // check and print the result or error
    if (res == IN3_OK) {
    printf("Latest block : \n%s\n", result);
        free(result);
    } else {
        printf("Error verifing the Latest block : \n%s\n", error);
        free(error);
    }
}

void get_block_api(in3_t* in3) {
    char *b2 = (char *) "{\"fromBlock\":\"0x95d0f0\",\"topics\":[\"0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef\",\"0x000000000000000000000000af62a9be3878d738a7ce32332c47bb401d9bc14c\"],\"toBlock\":\"0x95d0f1\"}";

        char b[30];
        sprintf(b, "{\"fromBlock\":\"0x%" PRIx64 "\"}", eth_blockNumber(in3) - 2);

        json_ctx_t* jopt = parse_json(b);

        // Create new filter with options
        size_t fid = eth_newFilter(in3, jopt);

        // Get logs
        eth_log_t* logs = NULL;
        in3_ret_t  ret  = eth_getFilterLogs(in3, fid, &logs);
        if (ret != IN3_OK) {
            printf("eth_getFilterLogs() failed [%d]\n", ret);
            return;
        }

        // print result
        while (logs) {
            eth_log_t* l = logs;
            printf("--------------------------------------------------------------------------------\n");
            printf("\tremoved: %s\n", l->removed ? "true" : "false");
            printf("\tlogId: %lu\n", l->log_index);
            printf("\tTxId: %lu\n", l->transaction_index);
            printf("\thash: ");
            ba_print(l->block_hash, 32);
            printf("\n\tnum: %" PRIu64 "\n", l->block_number);
            printf("\taddress: ");
            ba_print(l->address, 20);
            printf("\n\tdata: ");
            b_print(&l->data);
            printf("\ttopics[%lu]: ", l->topic_count);
            for (size_t i = 0; i < l->topic_count; i++) {
                printf("\n\t");
                ba_print(l->topics[i], 32);
            }
            printf("\n");
            logs = logs->next;
            free(l->data.data);
            free(l->topics);
            free(l);
        }
        eth_uninstallFilter(in3, fid);
        json_free(jopt);
}
