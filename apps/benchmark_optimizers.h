#include "tensor.h"
#include "math/abs.hpp"
#include "text/table.h"
#include "text/align.hpp"
#include "math/clamp.hpp"
#include "math/stats.hpp"
#include "thread/loopi.hpp"
#include "text/table_row_comp.h"

namespace benchmark
{
        using namespace nano;

        struct optimizer_stat_t
        {
                explicit optimizer_stat_t(const scalars_t& gthres = {1e-12, 1e-10, 1e-8, 1e-6})
                        :       m_gthres(gthres)
                {
                        assert(gthres.size() == 4);
                }

                scalars_t               m_gthres;       ///< thresholds for the convergence criterias
                nano::stats_t<scalar_t> m_crits;        ///< convergence criteria
                nano::stats_t<scalar_t> m_fail0s;       ///< #convergence failures
                nano::stats_t<scalar_t> m_fail1s;       ///< #convergence failures
                nano::stats_t<scalar_t> m_fail2s;       ///< #convergence failures
                nano::stats_t<scalar_t> m_fail3s;       ///< #convergence failures
                nano::stats_t<scalar_t> m_iters;        ///< #iterations
                nano::stats_t<scalar_t> m_fcalls;       ///< #function value calls
                nano::stats_t<scalar_t> m_gcalls;       ///< #gradient calls
                nano::stats_t<scalar_t> m_speeds;       ///< convergence speed (actually the average decrease ratio of the convergence criteria)
        };

        nano::stats_t<scalar_t> make_stats(const scalars_t& values, const scalars_t& flags)
        {
                assert(values.size() == flags.size());

                nano::stats_t<scalar_t> stats;
                for (size_t i = 0; i < values.size(); i ++)
                {
                        if (flags[i] >= 0.0)
                        {
                                stats(values[i]);
                        }
                }
                return stats;
        }

        void show_table(const std::string& table_name, const std::map<std::string, optimizer_stat_t>& ostats)
        {
                assert(!ostats.empty());

                const auto gthres = ostats.begin()->second.m_gthres;

                // show global statistics
                nano::table_t table(nano::align(table_name.empty() ? "optimizer" : table_name, 24));
                table.header() << "cost"
                               << "|grad|/|fval|"
                               << ("#>1e-" + nano::to_string(static_cast<size_t>(-std::log10(gthres[3]))))
                               << ("#>1e-" + nano::to_string(static_cast<size_t>(-std::log10(gthres[2]))))
                               << ("#>1e-" + nano::to_string(static_cast<size_t>(-std::log10(gthres[1]))))
                               << ("#>1e-" + nano::to_string(static_cast<size_t>(-std::log10(gthres[0]))))
                               << "#iters"
                               << "#fcalls"
                               << "#gcalls"
                               << "speed";

                for (const auto& it : ostats)
                {
                        const auto& name = it.first;
                        const auto& stat = it.second;

                        table.append(name) << static_cast<size_t>(stat.m_fcalls.avg() + 2 * stat.m_gcalls.avg())
                                           << stat.m_crits.avg()
                                           << static_cast<size_t>(stat.m_fail3s.sum())
                                           << static_cast<size_t>(stat.m_fail2s.sum())
                                           << static_cast<size_t>(stat.m_fail1s.sum())
                                           << static_cast<size_t>(stat.m_fail0s.sum())
                                           << static_cast<size_t>(stat.m_iters.avg())
                                           << static_cast<size_t>(stat.m_fcalls.avg())
                                           << static_cast<size_t>(stat.m_gcalls.avg())
                                           << stat.m_speeds.avg();
                }

                table.sort(nano::make_table_row_ascending_comp<scalar_t>(indices_t({2, 3, 0})));
                table.print(std::cout);
        }

        template
        <
                typename tscalar,
                typename tvector = typename nano::function_t<tscalar>::tvector,
                typename toptimizer,
                typename tostats
        >
        void benchmark_function(
                const nano::function_t<tscalar>& func, const std::vector<tvector>& x0s,
                const toptimizer& op, const std::string& name,
                const scalars_t& gthres,
                tostats& stats, tostats& gstats)
        {
                const auto trials = x0s.size();

                scalars_t crits(trials);
                scalars_t iters(trials);
                scalars_t fail0s(trials);
                scalars_t fail1s(trials);
                scalars_t fail2s(trials);
                scalars_t fail3s(trials);
                scalars_t fcalls(trials);
                scalars_t gcalls(trials);
                scalars_t speeds(trials);

                nano::pool_t pool;
                nano::loopi(trials, pool, [&] (size_t t)
                {
                        const auto& x0 = x0s[t];

                        const auto problem = func.problem();
                        const auto state0 = typename nano::function_t<tscalar>::tproblem::tstate(problem, x0);
                        const auto g0 = state0.convergence_criteria();

                        // optimize
                        const auto state = op(problem, x0);

                        const auto g = state.convergence_criteria();
                        const auto cost = state.m_fcalls + 2 * state.m_gcalls;
                        const auto speed = std::pow(g / g0, 1.0 / (1.0 + static_cast<scalar_t>(cost)));

                        // ignore out-of-domain solutions
                        if (func.is_valid(state.x))
                        {
                                // update stats
                                crits[t] = g;
                                iters[t] = static_cast<scalar_t>(state.m_iterations);
                                fail0s[t] = !state.converged(gthres[0]) ? 1.0 : 0.0;
                                fail1s[t] = !state.converged(gthres[1]) ? 1.0 : 0.0;
                                fail2s[t] = !state.converged(gthres[2]) ? 1.0 : 0.0;
                                fail3s[t] = !state.converged(gthres[3]) ? 1.0 : 0.0;
                                fcalls[t] = static_cast<scalar_t>(state.m_fcalls);
                                gcalls[t] = static_cast<scalar_t>(state.m_gcalls);
                                speeds[t] = speed;
                        }
                        else
                        {
                                // skip this from statistics!
                                crits[t] = -1.0;
                        }
                });

                // update per-problem statistics
                optimizer_stat_t& stat = stats[name];
                stat.m_gthres = gthres;
                stat.m_crits(make_stats(crits, crits));
                stat.m_iters(make_stats(iters, crits));
                stat.m_fail0s(make_stats(fail0s, crits));
                stat.m_fail1s(make_stats(fail1s, crits));
                stat.m_fail2s(make_stats(fail2s, crits));
                stat.m_fail3s(make_stats(fail3s, crits));
                stat.m_speeds(make_stats(speeds, crits));
                stat.m_fcalls(make_stats(fcalls, crits));
                stat.m_gcalls(make_stats(gcalls, crits));

                // update global statistics
                optimizer_stat_t& gstat = gstats[name];
                gstat.m_gthres = gthres;
                gstat.m_crits(stat.m_crits);
                gstat.m_iters(stat.m_iters);
                gstat.m_fail0s(stat.m_fail0s);
                gstat.m_fail1s(stat.m_fail1s);
                gstat.m_fail2s(stat.m_fail2s);
                gstat.m_fail3s(stat.m_fail3s);
                gstat.m_speeds(stat.m_speeds);
                gstat.m_fcalls(stat.m_fcalls);
                gstat.m_gcalls(stat.m_gcalls);
        }
}
