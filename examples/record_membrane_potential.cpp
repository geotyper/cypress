/*
 *  Cypress -- C++ Spiking Neural Network Simulation Framework
 *  Copyright (C) 2016  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file record_membrane_potential.cpp
 *
 * Records the membrane potential for a single neuron.
 *
 * @author Andreas Stöckel
 */

#include <iostream>
#include <fstream>

#include <cypress/cypress.hpp>

using namespace cypress;

int main(int argc, const char *argv[])
{
	if (argc != 2 && !NMPI::check_args(argc, argv)) {
		std::cout << "Usage: " << argv[0] << " <SIMULATOR>" << std::endl;
		return 1;
	}

	auto net =
	    Network()
	        .add_population<SpikeSourceArray>("source", 16, {50.0})
	        .add_population<IfFacetsHardware1>(
	            "target", 1, IfFacetsHardware1Parameters(),
	            IfFacetsHardware1Signals().record_v().record_spikes())
	        .add_connection("source", "target", Connector::all_to_all(0.015))
	        .run(argv[1], 100.0, argc, argv);

	auto tar = net.population<IfFacetsHardware1>("target");
	std::cout << "Target population produced "
	          << tar.signals().get_spikes().size() << " spikes" << std::endl;
	std::cout << tar.signals().get_v();

	return 0;
}