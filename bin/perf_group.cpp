#include <microscopes/common/random_fwd.hpp>
#include <microscopes/models/distributions.hpp>
#include <microscopes/models/noop.hpp>
#include <microscopes/common/recarray/dataview.hpp>
#include <microscopes/common/timer.hpp>

#include <random>
#include <iostream>

using namespace std;
using namespace distributions;
using namespace microscopes;
using namespace microscopes::common;
using namespace microscopes::common::recarray;

int
main(void)
{
  rng_t r(73);

  const size_t D = 1000;

  // create fake data
  bool data[D];
  for (size_t i = 0; i < D; i++)
    data[i] = bernoulli_distribution(0.5)(r);
  vector<runtime_type> types(D, runtime_type(TYPE_B));
  row_accessor acc( reinterpret_cast<const uint8_t *>(&data[0]), nullptr, &types);

  // no-op pointers
  vector<shared_ptr<models::hypers>> noop_shares;
  for (size_t i = 0; i < D; i++)
    noop_shares.emplace_back(models::noop_model().create_hypers());

  vector<shared_ptr<models::group>> noop_groups;
  for (const auto &px : noop_shares)
    noop_groups.emplace_back(px->create_group(r));

  // pointers
  vector<shared_ptr<models::hypers>> shares;
  for (size_t i = 0; i < D; i++)
    shares.emplace_back(models::distributions_model<BetaBernoulli>().create_hypers());

  vector<shared_ptr<models::group>> groups;
  for (const auto &px : shares)
    groups.emplace_back(px->create_group(r));

  // all in 1
  unique_ptr< models::distributions_hypers<BetaBernoulli> [] > combined_shares(
      new models::distributions_hypers<BetaBernoulli> [ D ]);

  unique_ptr< models::distributions_group<BetaBernoulli> []> combined_groups(
      new models::distributions_group<BetaBernoulli> [D]);
  for (size_t i = 0; i < D; i++)
    combined_groups[i].repr_.init(combined_shares[i].repr_, r);

  vector< models::hypers * > px_shares;
  for (size_t i = 0; i < D; i++)
    px_shares.emplace_back(&combined_shares[i]);

  vector< models::group * > px_groups;
  for (size_t i = 0; i < D; i++)
    px_groups.emplace_back(&combined_groups[i]);

  const size_t niters = 100000;

  {
    timer tt;
    for (size_t n = 0; n < niters; n++) {
      acc.reset();
      for (size_t i = 0; i < acc.nfeatures(); i++, acc.bump())
        noop_groups[i]->add_value(*noop_shares[i], acc.get(), r);
      acc.reset();
      for (size_t i = 0; i < acc.nfeatures(); i++, acc.bump())
        noop_groups[i]->remove_value(*noop_shares[i], acc.get(), r);
    }
    cout << "sec/iter: " << (tt.lap_ms() / float(niters)) << endl;
  }

  {
    timer tt;
    for (size_t n = 0; n < niters; n++) {
      acc.reset();
      for (size_t i = 0; i < acc.nfeatures(); i++, acc.bump())
        groups[i]->add_value(*shares[i], acc.get(), r);
      acc.reset();
      for (size_t i = 0; i < acc.nfeatures(); i++, acc.bump())
        groups[i]->remove_value(*shares[i], acc.get(), r);
    }
    cout << "sec/iter: " << (tt.lap_ms() / float(niters)) << endl;
  }

  {
    timer tt;
    for (size_t n = 0; n < niters; n++) {
      acc.reset();
      for (size_t i = 0; i < acc.nfeatures(); i++, acc.bump())
        px_groups[i]->add_value(*px_shares[i], acc.get(), r);
      acc.reset();
      for (size_t i = 0; i < acc.nfeatures(); i++, acc.bump())
        px_groups[i]->remove_value(*px_shares[i], acc.get(), r);
    }
    cout << "sec/iter: " << (tt.lap_ms() / float(niters)) << endl;
  }
  return 0;
}
