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

#include <cypress/core/network.hpp>
#include <cypress/core/backend.hpp>

namespace cypress {

namespace {
/**
 * Internally used parameter type with no parameters.
 */
class NullNeuronParameters final : public NeuronParametersBase {
public:
	NullNeuronParameters() : NeuronParametersBase(std::vector<float>{}) {}
};

/**
 * Internally used neuron type representing no neuron type.
 */
class NullNeuronType final : public NeuronType {
private:
	NullNeuronType() : NeuronType(0, "", {}, {}, {}, false, false) {}

public:
	using Parameters = NullNeuronParameters;

	static const NullNeuronType &inst()
	{
		static const NullNeuronType inst;
		return inst;
	}
};
}

/*
 * Class PopulationImpl
 */

class PopulationImpl {
private:
	/**
	 * Pointer at the actual network instance.
	 */
	Network *m_network;

	/**
	 * Size of the population.
	 */
	size_t m_size;

	/**
	 * Reference at the underlying neuron type.
	 */
	NeuronType const *m_type;

	/**
	 * Parameters shared by all neurons in the population.
	 */
	NeuronParametersBase m_parameters;

	/**
	 * Name of the population.
	 */
	std::string m_name;

public:
	/**
	 * Default constructor of the PopulationImpl class.
	 */
	PopulationImpl()
	    : m_network(nullptr),
	      m_size(0),
	      m_type(&NullNeuronType::inst()),
	      m_parameters(NullNeuronType::Parameters()),
	      m_name("")
	{
	}

	PopulationImpl(Network *network, size_t size, const NeuronType &type,
	               const NeuronParametersBase &parameters,
	               const std::string &name)
	    : m_network(network),
	      m_size(size),
	      m_type(&type),
	      m_parameters(parameters),
	      m_name(name)
	{
	}

	const NeuronType &type() const { return *m_type; }

	size_t size() const { return m_size; }

	const std::string &name() const { return m_name; }

	void name(const std::string &name)
	{
		m_name = name;  // TODO: Update network
	};

	Network &network() { return *m_network; }

	const NeuronParametersBase &parameters() const { return m_parameters; }

	bool homogeneous() const { return true; }
};

/*
 * Class PopulationBase
 */

const NeuronType &PopulationBase::type() const { return m_impl.type(); }

size_t PopulationBase::size() const { return m_impl.size(); }

const std::string &PopulationBase::name() const { return m_impl.name(); }

PopulationBase &PopulationBase::name(const std::string &name)
{
	m_impl.name(name);
	return *this;
}

Network &PopulationBase::network() { return m_impl.network(); }

bool PopulationBase::homogeneous() const { return m_impl.homogeneous(); }

/**
 * Class NetworkImpl
 */

class NetworkImpl {
private:
	std::vector<clone_ptr<PopulationImpl>> m_populations;

public:
	std::vector<PopulationImpl *> populations(const std::string &name,
	                                          const NeuronType *type)
	{
		std::vector<PopulationImpl *> res;
		for (auto &pop : m_populations) {
			if ((name.empty() || pop->name() == name) &&
			    (type == nullptr || &pop->type() == type)) {
				res.push_back(pop.ptr());
			}
		}
		return res;
	}

	PopulationImpl &create_population(Network *network, size_t size,
	                                  const NeuronType &type,
	                                  const NeuronParametersBase &params,
	                                  const std::string &name)
	{
		m_populations.emplace_back(
		    make_clone<PopulationImpl>(network, size, type, params, name));
		return *m_populations.back();
	}

	float duration() const
	{
		float res = 0.0;
		for (const auto &population : m_populations) {
			if (&population->type() == &SpikeSourceArray::inst()) {
				auto &params = population->parameters();
				if (params.size() > 0) {
					// The spike times are supposed to be sorted!
					res = std::max(res, params[params.size() - 1]);
				}
			}
		}
		return res;
	}
};

/**
 * Class Network
 */

Network::Network() : m_impl(make_clone<NetworkImpl>()) {}
Network::Network(const Network &) = default;
Network::Network(Network &&) noexcept = default;
Network &Network::operator=(const Network &) = default;
Network &Network::operator=(Network &&) = default;

Network::~Network()
{
	// Only required for the unique ptr to NetworkImpl
}

std::vector<PopulationImpl *> Network::populations(const std::string &name,
                                                   const NeuronType *type)
{
	return m_impl->populations(name, type);
}

float Network::duration() const { return m_impl->duration(); }

PopulationImpl &Network::create_population(size_t size, const NeuronType &type,
                                           const NeuronParametersBase &params,
                                           const std::string &name)
{
	return m_impl->create_population(this, size, type, params, name);
}

Network &Network::run(const Backend &backend, float duration)
{
	backend.run(*this, duration);
	return *this;
}
}
