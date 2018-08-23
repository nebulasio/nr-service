#pragma once

#include "blockchain/account_db.h"
#include "blockchain/transaction_db.h"
#include "common.h"
#include "graph/algo.h"

namespace neb {

struct rank_params_t {
  double m_a;
  double m_b;
  double m_c;
  double m_d;
  double m_mu;
  double m_lambda;
};

typedef std::shared_ptr<transaction_db_interface> transaction_db_ptr_t;
typedef std::shared_ptr<account_db_interface> account_db_ptr_t;

class nebulas_rank {
public:
  nebulas_rank();
  nebulas_rank(const transaction_db_ptr_t tdb_ptr,
               const account_db_ptr_t adb_ptr, const rank_params_t &rp,
               block_height_t start_block, block_height_t end_block);

  auto get_account_score_service() -> std::unordered_map<std::string, double>;

public:
  auto split_transactions_by_x_block_interval(
      const std::vector<transaction_info_t> &txs, int32_t block_interval = 128)
      -> std::vector<std::vector<transaction_info_t>>;

  void filter_empty_transactions_this_interval(
      std::vector<std::vector<transaction_info_t>> &txs);

  auto build_transaction_graphs(
      const std::vector<std::vector<transaction_info_t>> &txs)
      -> std::vector<transaction_graph_ptr>;

  auto get_normal_accounts(const std::vector<transaction_info_t> &txs)
      -> std::unordered_set<std::string>;

  auto get_account_balance_median(
      const std::unordered_set<std::string> &accounts,
      const std::vector<std::vector<transaction_info_t>> &txs,
      const account_db_ptr_t db_ptr) -> std::unordered_map<std::string, double>;

  auto get_account_weight(
      const std::unordered_map<std::string, in_out_val> &in_out_vals,
      const account_db_ptr_t db_ptr) -> std::unordered_map<std::string, double>;

  auto get_account_rank(
      const std::unordered_map<std::string, double> &account_median,
      const std::unordered_map<std::string, double> &account_weight,
      const rank_params_t &rp) -> std::unordered_map<std::string, double>;

private:
  template <class TransInfo>
  transaction_graph_ptr
  build_graph_from_transactions(const std::vector<TransInfo> &trans);

  block_height_t get_max_height_this_block_interval(
      const std::vector<transaction_info_t> &txs);

  double f_account_weight(double in_val, double out_val);

  double f_account_rank(double a, double b, double c, double d, double mu,
                        double lambda, double S, double R);

private:
  transaction_db_ptr_t m_tdb_ptr;
  account_db_ptr_t m_adb_ptr;
  rank_params_t m_rp;
  block_height_t m_start_block;
  block_height_t m_end_block;
  std::vector<transaction_info_t> m_txs;
}; // class nebulas_rank
} // namespace neb
