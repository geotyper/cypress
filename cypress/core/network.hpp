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
 * @file network.hpp
 *
 * This file contains the basic types a user should interact with when using
 * Cypress. These types are Network, Population, PopulationView and Neuron,
 * which represent various parts of spiking neural networks.
 *
 * Note that all classes defined here represent handles at an actual network --
 * copying these handles is cheap and will not clone the corresponding part
 * of the network. To clone a network, the Network::clone() method has to be
 * called.
 *
 * @author Andreas Stöckel
 */

#pragma once

#ifndef CYPRESS_CORE_NETWORK_HPP
#define CYPRESS_CORE_NETWORK_HPP

#include <cypress/core/exceptions.hpp>
#include <cypress/core/network_base.hpp>
#include <cypress/core/network_base_objects.hpp>

namespace cypress {

/*
 * Forward declarations
 */
class Network;
template <typename T>
class Population;
template <typename T>
class PopulationView;
template <typename T>
class Neuron;

/**
 * The Population class represents a set of neurons with the given type.
 *
 * @tparam T is the type of the neurons within the population.
 */
template <typename T>
class Population
    : public IterableMixin<Population<T>, Neuron<T>, Accessor>,
      public ViewableMixin<Population<T>, PopulationView<T>, Accessor>,
      public ParametersMixin<Population<T>, Accessor, typename T::Parameters>,
      public PopulationMixin<Population<T>, Accessor, typename T::Parameters>,
      public ConnectableMixin<Population<T>, Accessor> {
private:
	using PopulationMixin_ =
	    PopulationMixin<Population<T>, Accessor, typename T::Parameters>;
	using ParametersMixin_ =
	    ParametersMixin<Population<T>, Accessor, typename T::Parameters>;
	using IterableMixin_ = IterableMixin<Population<T>, Neuron<T>, Accessor>;
	using ViewableMixin_ =
	    ViewableMixin<Population<T>, PopulationView<T>, Accessor>;

	PopulationBase m_population;

public:
	using PopulationMixin_::size;
	using PopulationMixin_::parameters;
	using ParametersMixin_::parameters;
	using IterableMixin_::operator();
	using ViewableMixin_::operator();

	/**
	 * Creates a population as a handle pointing at an already existing
	 * population with the given population index.
	 *
	 * @param network is the network in which the population is located.
	 * @arapm pid is the population index of the already existing population
	 * in the referenced network.
	 */
	Population(const NetworkBase &network, PopulationIndex pid)
	    : m_population(network, pid)
	{
	}

	/**
	 * Creates a new population in the given network.
	 *
	 * @param network is the network in which the population should be
	 * instantiated.
	 * @param params is a vector of parameters with one entry for each neuron
	 * in the population. Pass an empty vector if the default parameters should
	 * be used. Set a single argument to share the same parameters between all
	 * neurons.
	 * @param size is the number of neurons in the population.
	 * @param name is the (optional) name of the population.
	 */
	Population(Network &network, size_t size,
	           const typename T::Parameters &params, const char *name = "");

	/**
	 * Creates a new population in the given network.
	 *
	 * @param network is the network in which the population should be
	 * instantiated.
	 * @param params is a vector of parameters with one entry for each neuron
	 * in the population. Pass an empty vector if the default parameters should
	 * be used. Set a single argument to share the same parameters between all
	 * neurons.
	 * @param size is the number of neurons in the population.
	 * @param name is the (optional) name of the population.
	 */
	Population(Network &network, size_t size,
	           std::initializer_list<typename T::Parameters> params,
	           const char *name = "");

	/**
	 * This class can be implicitly converted to PopulationBase, loosing the
	 * neuron type information.
	 */
	operator PopulationBase() const { return m_population; }
	operator const PopulationBase &() const { return m_population; }
	operator PopulationBase &() { return m_population; }

	/**
	 * Returns the type of the population.
	 *
	 * @return a reference at the neuron type descriptor.
	 */
	const NeuronType &type() const { return T::inst(); }

	/**
	 * Returns a handle pointing at the network this population is located in.
	 *
	 * @return a Network object representing the network this population is
	 * located in.
	 */
	Network network() const;

	/**
	 * Returns the population index of this population.
	 *
	 * @return the population index of this population.
	 */
	PopulationIndex pid() const { return m_population.pid(); }
};

/**
 * The PopulationView class represents a subset of neurons in a Population.
 *
 * @tparam T is the neuron type.
 */
template <typename T>
class PopulationView
    : public IterableMixin<PopulationView<T>, Neuron<T>, Accessor>,
      public ViewableMixin<PopulationView<T>, PopulationView<T>, Accessor>,
      public ParametersMixin<Population<T>, Accessor, typename T::Parameters>,
      public ConnectableMixin<PopulationView<T>, Accessor> {
private:
	using IterableMixin_ =
	    IterableMixin<PopulationView<T>, Neuron<T>, Accessor>;
	using ViewableMixin_ =
	    ViewableMixin<PopulationView<T>, PopulationView<T>, Accessor>;

	PopulationViewBase m_view;

public:
	using IterableMixin_::operator();
	using ViewableMixin_::operator();

	/**
	 * Creates a population view as a handle pointing at an already existing
	 * population with the given population index and the specified range of
	 * neuron indices.
	 *
	 * @param network is the network in which the population is located.
	 * @param pid is the population index of the already existing population
	 * in the referenced network.
	 * @param nid0 is the index of the first neuron in the population that
	 * can be accessed in this view.
	 * @param nid1 is the index of the last-plus-one neuron in the population
	 * that can be accessed in this view.
	 */
	PopulationView(const NetworkBase &network, PopulationIndex pid,
	               NeuronIndex nid0, NeuronIndex nid1)
	    : m_view(network, pid, nid0, nid1)
	{
	}

	/**
	 * This class can be implicitly converted to PopulationBase, loosing the
	 * neuron type information.
	 */
	operator PopulationViewBase() const { return m_view; }
	operator const PopulationViewBase &() const { return m_view; }
	operator PopulationViewBase &() { return m_view; }

	/**
	 * Returns the type of the population.
	 *
	 * @return a reference at the neuron type descriptor.
	 */
	const NeuronType &type() const { return T::inst(); }

	/**
	 * Returns a handle pointing at the network this population is located in.
	 *
	 * @return a Network object representing the network this population is
	 * located in.
	 */
	Network network() const;

	/**
	 * Returns the population index of this population.
	 *
	 * @return the population index of this population.
	 */
	PopulationIndex pid() const { return m_view.pid(); }

	/**
	 * Returns the index of the first neuron belonging to this PopulationView.
	 */
	NeuronIndex nid_begin() const { return m_view.nid_begin(); }

	/**
	 * Returns the index of the last-plus-one neuron belonging to this
	 * PopulationView.
	 */
	NeuronIndex nid_end() const { return m_view.nid_end(); }
};

/**
 * The NeuronBase class represents a generic neuron of any type. If possible,
 * prefer the usage of Neuron<T>.
 */
template <typename T>
class Neuron
    : public IterableMixin<Neuron<T>, Neuron<T>, Accessor>,
      public ViewableMixin<Neuron<T>, PopulationViewBase, Accessor>,
      public ParametersMixin<Neuron<T>, Accessor, typename T::Parameters>,
      public NeuronMixin<Neuron<T>, Accessor, typename T::Parameters>,
      public ConnectableMixin<Neuron<T>, Accessor> {
private:
	using NeuronMixin_ =
	    NeuronMixin<Neuron<T>, Accessor, typename T::Parameters>;
	using ParametersMixin_ =
	    ParametersMixin<Neuron<T>, Accessor, typename T::Parameters>;
	using IterableMixin_ = IterableMixin<Neuron<T>, Neuron<T>, Accessor>;
	using ViewableMixin_ =
	    ViewableMixin<Neuron<T>, PopulationViewBase, Accessor>;

	NeuronBase m_neuron;

public:
	using NeuronMixin_::parameters;
	using ParametersMixin_::parameters;
	using IterableMixin_::operator();
	using ViewableMixin_::operator();

	/**
	 * Creates a Neuron object pointing at the nid-th neuron in the given
	 * neuron population.
	 *
	 * @param parent is the neuron population or population view the neuron is
	 * located in.
	 * @param nid is the absolute index of the neuron in its population.
	 */
	template <typename Parent>
	Neuron(const Parent &parent, NeuronIndex nid)
	    : m_neuron(parent, nid)
	{
	}

	/**
	 * This class can be implicitly converted to NeuronBase, loosing the
	 * neuron type information.
	 */
	operator NeuronBase() const { return m_neuron; }
	operator const NeuronBase &() const { return m_neuron; }
	operator NeuronBase &() { return m_neuron; }

	/**
	 * Returns the type of the neuron.
	 */
	const NeuronType &type() const { return T::inst(); }

	/**
	 * Returns a reference at the network instance the neuron is part of.
	 */
	Network network() const;

	/**
	 * Returns a handle pointing at the population this neuron is located in.
	 *
	 * @return a Population<T> object pointing at the population this neuron
	 * is located in.
	 */
	Population<T> population() const
	{
		return Population<T>(m_neuron.network(), pid());
	}

	/**
	 * Returns the population index of the population this neuron belongs to.
	 */
	PopulationIndex pid() const { return m_neuron.pid(); }

	/**
	 * Returns the index of this neuron within its population. Note that this
	 * index is never relative to the start index of a population view but
	 * always relative to the population itself.
	 */
	NeuronIndex nid() const { return m_neuron.nid(); }
};

/**
 * Class representing the entire spiking neural network.
 */
class Network : public NetworkBase {
private:
	template <typename T>
	friend class Population;

	template <typename T>
	size_t create_population_index(
	    size_t size, const std::vector<typename T::Parameters> &params,
	    const std::string &name = std::string())
	{
		return NetworkBase::create_population_index(
		    size, T::inst(),
		    std::vector<NeuronParametersBase>(params.begin(), params.end()),
		    name);
	}

public:
	using NetworkBase::populations;
	using NetworkBase::population;

	Network() = default;
	Network(const Network &) = default;
	Network(Network &&) noexcept = default;
	Network &operator=(const Network &) = default;
	Network &operator=(Network &&) = default;
	~Network() = default;

	/**
	 * Allow creation of a network instance from a NetworkBase instance.
	 */
	Network(const NetworkBase &network) : NetworkBase(network) {}

	/**
	 * Allow assigning a NetworkBase instance to a Network instance.
	 */
	Network &operator=(const NetworkBase &o)
	{
		*this = Network(o);
		return *this;
	}

	/**
	 * Creates a new neuron population with the given size and name.
	 *
	 * @param size is the number of neurons in the population.
	 * @param name is the (optional) name of the population. A C-style string
	 * is used here to prevent iterpretation of an initializer list as string.
	 */
	template <typename T>
	Population<T> create_population(size_t size, const char *name ="")
	{
		return Population<T>(*this, size, {}, name);
	}

	template <typename T>
	Population<T> create_population(
	    size_t size, const typename T::Parameters &params,
	    const char *name ="")
	{
		return Population<T>(*this, size, params, name);
	}

	template <typename T>
	Population<T> create_population(
	    size_t size, std::initializer_list<typename T::Parameters> params,
	    const char *name = "")
	{
		return Population<T>(*this, size, params, name);
	}

	/**
	 * Returns population objects pointing at the population with the given
	 * name.
	 *
	 * @tparam T is the neuron type of the population that should be returned.
	 * @param name is the name of the population that is being searched for. If
	 * empty, all populations are matched.
	 * @return a vector of population objects.
	 */
	template <typename T>
	std::vector<Population<T>> populations(
	    const std::string &name = std::string())
	{
		std::vector<Population<T>> res;
		for (const PopulationBase &p : populations(name, T::inst())) {
			res.emplace_back(*this, p.pid());
		}
		return res;
	}

	/**
	 * Returns a Population object pointing at the last created population with
	 * the given name. If no such population exists, an exception is thrown.
	 *
	 * @tparam T is the neuron type of the population that should be returned.
	 * @param name is the name of the population that should be looked up. If
	 * empty, the last created population of the given type is returned.
	 * @return a population handle pointing at the requested population.
	 */
	template <typename T>
	Population<T> population(const std::string &name = std::string())
	{
		auto pops = populations<T>(name);
		if (pops.empty()) {
			throw NoSuchPopulationException(
			    std::string("Population of type \"") + T::inst().name +
			    "\" with name \"" + name + "\" does not exist");
		}
		return pops.back();
	}
};

/*
 * Out-of-class function definitions.
 */

template <typename T>
inline Population<T>::Population(Network &network, size_t size,
                                 const typename T::Parameters &params,
                                 const char *name)
    : Population(network,
                 network.create_population_index<T>(size, {params}, name))
{
}

template <typename T>
inline Population<T>::Population(
    Network &network, size_t size,
    std::initializer_list<typename T::Parameters> params, const char *name)
    : Population(network,
                 network.create_population_index<T>(size, params, name))
{
}

template <typename T>
inline Network Population<T>::network() const
{
	return Network(m_population.network());
}

template <typename T>
inline Network PopulationView<T>::network() const
{
	return Network(m_view.network());
}

template <typename T>
inline Network Neuron<T>::network() const
{
	return Network(m_neuron.network());
}
}

#endif /* CYPRESS_CORE_NETWORK_HPP */

