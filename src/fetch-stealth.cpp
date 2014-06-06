/**
 * Copyright (c) 2011-2014 sx developers (see AUTHORS)
 *
 * This file is part of sx.
 *
 * sx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <obelisk/obelisk.hpp>
#include <sx/command/fetch-stealth.hpp>
#include <sx/utility/config.hpp>
#include <sx/obelisk.hpp>
#include <sx/utility/console.hpp>

using namespace bc;
using namespace sx;
using namespace sx::extensions;
using std::placeholders::_1;
using std::placeholders::_2;

// TODO: this should be a member of sx::extensions::fetch_stealth,
// otherwise concurrent test execution will collide on shared state.
static bool node_stopped = false;

// TODO: node_stopped should be passed here via closure
// or by converting this to a member function.
static void stealth_fetched(const std::error_code& ec,
    const blockchain::stealth_list& stealth_results)
{
    if (ec)
        log_error() << "Stealth fetch failed: " << ec.message();
    else
        for (const blockchain::stealth_row& row: stealth_results)
            std::cout << "ephemkey: " << row.ephemkey
                << " address: " << row.address.encoded()
                << " tx_hash: " << row.transaction_hash << std::endl;

    node_stopped = true;
}

console_result fetch_stealth::invoke(int argc, const char* argv[])
{
    if (!validate_argument_range(argc, example(), 1, 3))
        return console_result::failure;

    std::string prefix_str(get_arg(argc, argv));
    stealth_prefix prefix(prefix_str);

    // TODO: create console util for this idiom (numeric arg read).
    size_t height = 0;
    if (argc > 2&&!parse(height, argv[2]))
    {
        std::cerr << "sx: Invalid height value specified." << std::endl;
        return console_result::failure;
    }

    OBELISK_FULLNODE(pool, fullnode);
    fullnode.blockchain.fetch_stealth(prefix, 
        std::bind(stealth_fetched, _1, _2), height);
    poll(fullnode, pool, node_stopped);

    return console_result::okay;
}