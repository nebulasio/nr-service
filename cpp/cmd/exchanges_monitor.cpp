#include "blockchain.h"
#include "utils.h"

DEFINE_int32(start_ts, 0, "the first day end timestamp");
DEFINE_int32(end_ts, 1, "the last day end timestamp");

typedef neb::transaction_db_interface transaction_db_t;
typedef std::shared_ptr<transaction_db_t> tdb_ptr_t;
typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int32_t start_ts = FLAGS_start_ts;
  int32_t end_ts = FLAGS_end_ts;

  tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));

  time_t seconds_of_day = 24 * 60 * 60;
  time_t seconds_of_ten_minute = 10 * 60;

  std::vector<std::pair<std::string, std::string>> name_and_addr(
      {{"binance", "n1NCdn2vo1vz2didNfnvxPaAPZbh634CLqM"},
       {"huobi", "n1KxWR8ycXg7Kb9CPTtNjTTEpvka269PniB"},
       {"okex", "n1M6ca8bB3VZyWBryeDBX42kHV9Q8yGXsSP"},
       {"bbaex", "n1NRCR4auPGK8yJ11b3GqhBKe2w1mmUioim"},
       {"ceo.bi", "n1bvE3Zs4H8gE1QTD1dCS5Gx4hrfLoRA2oW"},
       {"allcoin", "n1aafQBY9V3HVKCKLwBYJDxrF61RMdJNxAR"},
       {"lbank", "n1Je6AWHKtrLEEPXeAe74fCzqqmzLLS49wm"},
       {"bcex", "n1KWv3XujZEqWamhd8Nh3cDHmhhLeZJKQko"},
       {"gate.io", "n1Ugq21nif8BQ8uw81SwXHK6DHqeTEmPRhj"}});

  for (auto &it : name_and_addr) {
    std::ofstream file(it.first + ".csv");
    file << "date,balance\n";

    for (auto ts = start_ts; ts < end_ts; ts += seconds_of_day) {
      std::string date = neb::time_utils::time_t_to_date(ts);
      auto it_txs_in_start_last_minute =
          tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
              std::to_string(ts - seconds_of_ten_minute), std::to_string(ts));
      auto txs_in_start_last_minute = *it_txs_in_start_last_minute;

      if (!txs_in_start_last_minute.empty()) {
        neb::block_height_t height =
            txs_in_start_last_minute.back().template get<::neb::height>();
        std::string balance =
            neb::nebulas::get_account_state(it.second, height).first;
        file << date << ',' << std::stod(balance) / pow(10, 18) << '\n';

        LOG(INFO) << it.first << ',' << date << ',' << balance << ',' << height;
      }
    }
    file.close();
  }

  return 0;
}
