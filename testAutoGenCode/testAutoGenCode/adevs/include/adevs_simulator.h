/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
#ifndef __adevs_simulator_h_
#define __adevs_simulator_h_
#include "adevs_abstract_simulator.h"
#include "adevs_models.h"
#include "adevs_event_listener.h"
#include "adevs_sched.h"
#include "adevs_bag.h"
#include "adevs_set.h"
#include "object_pool.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace adevs
{

/**
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods throw adevs::exception objects if any of the DEVS model
 * constraints are violated (i.e., a negative time advance, a model
 * attempting to send an input directly to itself, or coupled Mealy
 * type systems).
 */
template <class X, class T = double> class Simulator:
	public AbstractSimulator<X,T>,
	private Schedule<X,T>::ImminentVisitor
{
	public:
		/**
		 * Create a simulator for a model. The simulator
		 * constructor will fail and throw an adevs::exception if the
		 * time advance of any component atomic model is less than zero.
		 * @param model The model to simulate
		 */
		Simulator(Devs<X,T>* model):
			AbstractSimulator<X,T>(),
			Schedule<X,T>::ImminentVisitor(),
			io_up_to_date(false)
		{
			schedule(model,adevs_zero<T>());
		}
		/**
		 * Get the model's next event time
		 * @return The absolute time of the next event
		 */
		T nextEventTime()
		{
			return sched.minPriority();
		}
		/// Execute the simulation cycle at time nextEventTime()
		void execNextEvent()
		{
			computeNextState();
		}
		/// Execute until nextEventTime() > tend
		void execUntil(T tend)
		{
			while (nextEventTime() <= tend 
                   && nextEventTime() < adevs_inf<T>()) {
				execNextEvent();
            }
		}
		/**
		 * Compute the output values of the imminent component models 
		 * if these values have not already been computed.  This will
		 * notify registered EventListeners as the outputs are produced. 
		 */
		void computeNextOutput();
		/**
		 * Compute the output value of the model in response to an input
		 * at some time in lastEventTime() <= t <= nextEventTime().
		 * This will notify registered EventListeners as the outputs
		 * are produced. If this is the first call since the prior
		 * state change with the given t, then the new output is computed.
		 * Subsequent calls for the same time t simply
		 * append to the input already supplied at time t.
		 * @param input A bag of (input target,value) pairs
		 * @param t The time at which the input takes effect
		 */
		void computeNextOutput(Bag<Event<X,T> >& input, T t);
		/**
		 * Apply the bag of inputs at time t and then compute the next model
		 * states. Requires that lastEventTime() <= t <= nextEventTime().
		 * This, in effect, implements the state transition function of 
		 * the resultant model. If the output has already been computed
		 * at time t, then the new input at t is simply appended to the
		 * prior input. Otherwise, the old results are discarded and input
		 * is calculated at the given time.
		 * @param input A bag of (input target,value) pairs
		 * @param t The time at which the input takes effect
		 */
		void computeNextState(Bag<Event<X,T> >& input, T t);
		/**
		 * Compute the next state at the time at the time t and with
		 * input supplied at the prior call to computeNextOutput
		 * assuming no computeNextState has intervened. Assumes
		 * t = nextEventTime() and input an empty bag if there was
		 * no prior call to computeNextOutput.
		 */
		void computeNextState();
		/**
		 * Deletes the simulator, but leaves the model intact. The model must
		 * exist when the simulator is deleted.  Delete the model only after
		 * the Simulator is deleted.
		 */
		~Simulator();
		/**
		 * Assign a model to the simulator. This has the same effect as passing
		 * the model to the constructor.
		 */
		void addModel(Atomic<X,T>* model) 
		{
			schedule(model,adevs_zero<T>());
		}
	private:
		// Bogus input bag for execNextEvent() method
		Bag<Event<X,T> > bogus_input;
		// The event schedule
		Schedule<X,T> sched;
		// List of models that are imminent or activated by input
		Bag<Atomic<X,T>*> activated;
		// Mealy systems that we need to process
		bool allow_mealy_input, io_up_to_date;
		T io_time;
		Bag<MealyAtomic<X,T>*> mealy;
		// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X,T> > > recv_pool;
		// Sets for computing structure changes.
		Bag<Devs<X,T>*> added;
		Bag<Devs<X,T>*> removed;
		Set<Devs<X,T>*> next;
		Set<Devs<X,T>*> prev;
		// Model transition functions are evaluated from the bottom up!
		struct bottom_to_top_depth_compare
		{
			bool operator()(const Network<X,T>* m1, const Network<X,T>* m2) const
			{
				unsigned long int d1 = 0, d2 = 0;
				// Compute depth of m1
				const Network<X,T>* m = m1->getParent();
				while (m != NULL) 
				{
					d1++;
					m = m->getParent();
				}
				// Compute depth of m2
				m = m2->getParent();
				while (m != NULL)
				{
					d2++;
					m = m->getParent();
				}
				// Models at the same depth are sorted by name
				if (d1 == d2) return m1 < m2;
				// Otherwise, sort by depth
				return d1 > d2;
			}
		};
		struct top_to_bottom_depth_compare
		{
			bool operator()(const Devs<X,T>* m1, const Devs<X,T>* m2) const
			{
				unsigned long int d1 = 0, d2 = 0;
				// Compute depth of m1
				const Network<X,T>* m = m1->getParent();
				while (m != NULL) 
				{
					d1++;
					m = m->getParent();
				}
				// Compute depth of m2
				m = m2->getParent();
				while (m != NULL)
				{
					d2++;
					m = m->getParent();
				}
				// Models at the same depth are sorted by name
				if (d1 == d2) return m1 < m2;
				// Otherwise, sort by depth
				return d1 < d2;
			}
		};
		std::set<Network<X,T>*,bottom_to_top_depth_compare> model_func_eval_set;
		std::set<Devs<X,T>*,top_to_bottom_depth_compare> sorted_removed;
		/**
		 * Recursively add the model and its elements to the schedule 
		 * using t as the time of last event.
		 */
		void schedule(Devs<X,T>* model, T t);
		/// Route an event generated by the source model contained in the parent model.
		void route(Network<X,T>* parent, Devs<X,T>* src, X& x);
		/**	
		 * Add an input to the input bag of an an atomic model. If the 
		 * model is not already active , then this method adds the model to
		 * the activated bag.
		 */
		void inject_event(Atomic<X,T>* model, X& value);
		/**
		 * Recursively remove a model and its components from the schedule 
		 * and the imminent/activated bags
		 */
		void unschedule_model(Devs<X,T>* model);
		/**
		 * Delete any thing in the output bag, and return the input
		 * and output bags to the pools.
		 * Recursively clean up network model components.
		 */
		void clean_up(Devs<X,T>* model);
		/**
		 * Construct the complete descendant set of a network model and store it in s.
		 */
		void getAllChildren(Network<X,T>* model, Set<Devs<X,T>*>& s);
		/**
		 * Visit method inhereted from ImminentVisitor
		 */
		void visit(Atomic<X,T>* model);
};

template <class X, class T>
void Simulator<X,T>::visit(Atomic<X,T>* model)
{
	assert(model->y == NULL);
	// Mealy models are processed after the Moore models
	if (model->typeIsMealyAtomic() != NULL)
	{
		model->typeIsMealyAtomic()->imm = true;
		assert(model->y == NULL);
		// May be in the mealy list because of a route call
		if (model->x == NULL)
			mealy.insert(model->typeIsMealyAtomic());
		return;
	}
	model->y = io_pool.make_obj();
	// Put it in the active list if it is not already there
	if (model->x == NULL)
		activated.insert(model);
	// Compute output functions and route the events. The bags of output
	// are held for garbage collection at a later time.
	model->output_func(*(model->y));
	// Route each event in y
	for (typename Bag<X>::iterator y_iter = model->y->begin(); 
		y_iter != model->y->end(); y_iter++)
	{
		route(model->getParent(),model,*y_iter);
	}
}

template <class X, class T>
void Simulator<X,T>::computeNextOutput(Bag<Event<X,T> >& input, T t)
{
	// Undo any prior output calculation at another time
	if (io_up_to_date && !(io_time == t))
	{
		typename Bag<Atomic<X,T>*>::iterator iter;
		for (iter = activated.begin(); iter != activated.end(); iter++)
		{
			clean_up(*iter);
		}
		activated.clear();
	}
	// Get the imminent Moore models from the schedule if we have not
	// already done so.
	allow_mealy_input = true;
	if (t == sched.minPriority() && !io_up_to_date)
		sched.visitImminent(this);
	// Apply the injected inputs
	for (typename Bag<Event<X,T> >::iterator iter = input.begin(); 
		iter != input.end(); iter++)
	{
		Atomic<X,T>* amodel = (*iter).model->typeIsAtomic();
		if (amodel != NULL)
		{
			inject_event(amodel,(*iter).value);
		}
		else
		{
			route((*iter).model->typeIsNetwork(),(*iter).model,(*iter).value);
		}
	}
	// Only Moore models can influence Mealy models. 
	allow_mealy_input = false;
	// Iterate over activated Mealy models to calculate their output
	for (typename Bag<MealyAtomic<X,T>*>::iterator m_iter = mealy.begin();
		m_iter != mealy.end(); m_iter++)
	{
		MealyAtomic<X,T> *model = *m_iter;
		assert(model->y == NULL);
		model->y = io_pool.make_obj();
		// Put it in the active set if it is not already there
		if (model->x == NULL)
			activated.insert(model);
		// Compute output functions and route the events. The bags of output
		// are held for garbage collection at a later time.
		if (model->imm) // These are the imminent Mealy models
		{
			if (model->x == NULL)
				model->typeIsAtomic()->output_func(*(model->y));
			else
				model->output_func(*(model->x),*(model->y));
		}
		else
		{
			assert(model->x != NULL);
			// These are the Mealy models activated by input
			model->output_func(sched.minPriority()-model->tL,*(model->x),*(model->y));
		}
	}
	// Translate Mealy output to inputs for Moore models. The route method
	// will throw an exception if an event is sent to a Mealy model.
	for (typename Bag<MealyAtomic<X,T>*>::iterator m_iter = mealy.begin();
		m_iter != mealy.end(); m_iter++)
	{
		MealyAtomic<X,T> *model = *m_iter;
		// Route each event in y
		for (typename Bag<X>::iterator y_iter = model->y->begin(); 
			y_iter != model->y->end(); y_iter++)
		{
			route(model->getParent(),model,*y_iter);
		}
	}
	mealy.clear();
	// Record the time of the input
	io_up_to_date = true;
	io_time = t;

}

template<class X, class T>
void Simulator<X,T>::computeNextOutput()
{
	computeNextOutput(bogus_input,sched.minPriority());
}

template <class X, class T>
void Simulator<X,T>::computeNextState(Bag<Event<X,T> >& input, T t)
{
	computeNextOutput(input,t);
	assert(io_time == t && io_up_to_date);
	computeNextState();
}

template <class X, class T>
void Simulator<X,T>::computeNextState()
{
	if (!io_up_to_date)
		computeNextOutput();
	io_up_to_date = false;
	T t = io_time, tQ = io_time + adevs_epsilon<T>();
	/*
	 * Compute the states of atomic models.  Store Network models that 
	 * need to have their model transition function evaluated in a
	 * special container that will be used when the structure changes are
	 * computed.
	 */
	#ifdef _OPENMP
	#pragma omp parallel for
	#endif
	for (unsigned i = 0; i < activated.size(); i++)
	{
		Atomic<X,T>* model = activated[i];
		// Internal event if no input
		if (model->x == NULL)
			model->delta_int();
		// Confluent event if model is imminent and has input
		else if (
				(model->typeIsMealyAtomic() == NULL && model->y != NULL)
				|| (model->typeIsMealyAtomic() != NULL && model->typeIsMealyAtomic()->imm)
			)
			model->delta_conf(*(model->x));
		// External event if model is not imminent and has input
		else
			model->delta_ext(t-model->tL,*(model->x));
		// Notify listeners 
		this->notify_state_listeners(model,tQ);
		// Check for a model transition
		if (model->model_transition() && model->getParent() != NULL)
		{
			#ifdef _OPENMP
			#pragma omp critical
			#endif
			model_func_eval_set.insert(model->getParent());
		}
		// Adjust position in the schedule
		schedule(model,tQ);
	}
	/**
	 * The new states are in effect at t + eps so advance t
	 */
	t = tQ;
	/**
	 * Compute model transitions and build up the prev (pre-transition)
	 * and next (post-transition) component sets. These sets are built
	 * up from only the models that have the model_transition function
	 * evaluated.
	 */
	if (model_func_eval_set.empty() == false)
	{
		while (!model_func_eval_set.empty())
		{
			Network<X,T>* network_model = *(model_func_eval_set.begin());
			model_func_eval_set.erase(model_func_eval_set.begin());
			getAllChildren(network_model,prev);
			if (network_model->model_transition() &&
					network_model->getParent() != NULL)
			{
				model_func_eval_set.insert(network_model->getParent());
			}
			getAllChildren(network_model,next);
		}
		// Find the set of models that were added.
		set_assign_diff(added,next,prev);
		// Find the set of models that were removed
		set_assign_diff(removed,prev,next);
		// Intersection of added and removed is always empty, so no need to look
		// for models in both (an earlier version of the code did this).
		next.clear();
		prev.clear();
		/** 
		 * The model adds are processed first.  This is done so that, if any
		 * of the added models are components something that was removed at
		 * a higher level, then the models will not have been deleted when
		 * trying to schedule them.
		 */
		for (typename Bag<Devs<X,T>*>::iterator iter = added.begin(); 
			iter != added.end(); iter++)
		{
			schedule(*iter,t);
		}
		// Done with the additions
		added.clear();
		// Remove the models that are in the removed set.
		for (typename Bag<Devs<X,T>*>::iterator iter = removed.begin(); 
			iter != removed.end(); iter++)
		{
			clean_up(*iter);
			unschedule_model(*iter);
			// Add to a sorted remove set for deletion
			sorted_removed.insert(*iter); 
		}
		// Done with the unsorted remove set
		removed.clear();
		// Delete the sorted removed models
		while (!sorted_removed.empty())
		{
			// Get the model to erase
			Devs<X,T>* model_to_remove = *(sorted_removed.begin());
			// Remove the model
			sorted_removed.erase(sorted_removed.begin());
			/**
			 * If this model has children, then remove them from the 
			 * deletion set. This will avoid double delete problems.
			 */
			if (model_to_remove->typeIsNetwork() != NULL)
			{
				getAllChildren(model_to_remove->typeIsNetwork(),prev);
				typename Set<Devs<X,T>*>::iterator iter = prev.begin();
				for (; iter != prev.end(); iter++)
					sorted_removed.erase(*iter);
				prev.clear();
			}
			// Delete the model and its children
			delete model_to_remove;
		}
		// Removed sets should be empty now
		assert(prev.empty());
		assert(sorted_removed.empty());
	} // End of the structure change
	// Cleanup and reschedule models that changed state in this iteration
	// and survived the structure change phase.
	for (typename Bag<Atomic<X,T>*>::iterator iter = activated.begin(); 
		iter != activated.end(); iter++)
	{
		clean_up(*iter);
	}
	// Empty the bags
	activated.clear();
}

template <class X, class T>
void Simulator<X,T>::clean_up(Devs<X,T>* model)
{
	Atomic<X,T>* amodel = model->typeIsAtomic();
	if (amodel != NULL)
	{
		if (amodel->x != NULL)
		{
			amodel->x->clear();
			io_pool.destroy_obj(amodel->x);
			amodel->x = NULL;
		}
		if (amodel->y != NULL)
		{
			amodel->gc_output(*(amodel->y));
			amodel->y->clear();
			io_pool.destroy_obj(amodel->y);
			amodel->y = NULL;
		}
		if (amodel->typeIsMealyAtomic() != NULL)
		{
			amodel->typeIsMealyAtomic()->imm = false;
		}
	}
	else
	{
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		for (typename Set<Devs<X,T>*>::iterator iter = components.begin();
		iter != components.end(); iter++)
		{
			clean_up(*iter);
		}
	}
}

template <class X, class T>
void Simulator<X,T>::unschedule_model(Devs<X,T>* model)
{
	if (model->typeIsAtomic() != NULL)
	{
		sched.schedule(model->typeIsAtomic(),adevs_inf<T>());
		activated.erase(model->typeIsAtomic());
	}
	else
	{
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		for (typename Set<Devs<X,T>*>::iterator iter = components.begin();
		iter != components.end(); iter++)
		{
			unschedule_model(*iter);
		}
	}
}

template <class X, class T>
void Simulator<X,T>::schedule(Devs<X,T>* model, T t)
{
	Atomic<X,T>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		a->tL = t;
		T dt = a->ta();
		if (dt == adevs_inf<T>())
		{
			#ifdef _OPENMP
			#pragma omp critical
			#endif
			sched.schedule(a,adevs_inf<T>());
		}
		else
		{
			T tNext = a->tL+dt;
			if (tNext < a->tL)
			{
				exception err("Negative time advance",a);
				throw err;
			}
			#ifdef _OPENMP
			#pragma omp critical
			#endif
			sched.schedule(a,tNext);
		}
	}
	else
	{
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X,T>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			schedule(*iter,t);
		}
	}
}

template <class X, class T>
void Simulator<X,T>::inject_event(Atomic<X,T>* model, X& value)
{
	// If this is a Mealy model, add it to the list of models that
	// will need their input calculated
	if (model->typeIsMealyAtomic())
	{
		if (allow_mealy_input)
		{
			assert(model->y == NULL);
			// Add it to the list of its not already there
			if (model->x == NULL && !model->typeIsMealyAtomic()->imm)
				mealy.insert(model->typeIsMealyAtomic());
		}
		else
		{
			exception err("Mealy model coupled to a Mealy model",model);
			throw err;
		}
	}
	// Add the output to the model's bag of output to be processed
	if (model->x == NULL)
	{
		if (model->y == NULL)
			activated.insert(model);
		model->x = io_pool.make_obj();
	}
	model->x->insert(value);
}

template <class X, class T>
void Simulator<X,T>::route(Network<X,T>* parent, Devs<X,T>* src, X& x)
{
	// Notify event listeners if this is an output event
	if (parent != src)
		this->notify_output_listeners(src,x,sched.minPriority());
	// No one to do the routing, so return
	if (parent == NULL) return;
	// Compute the set of receivers for this value
	Bag<Event<X,T> >* recvs = recv_pool.make_obj();
	parent->route(x,src,*recvs);
	// Deliver the event to each of its targets
	Atomic<X,T>* amodel = NULL;
	typename Bag<Event<X,T> >::iterator recv_iter = recvs->begin();
	for (; recv_iter != recvs->end(); recv_iter++)
	{
		/**
		 * If the destination is an atomic model, add the event to the IO bag
		 * for that model and add model to the list of activated models
		 */
		amodel = (*recv_iter).model->typeIsAtomic();
		if (amodel != NULL)
		{
			inject_event(amodel,(*recv_iter).value);
		}
		// if this is an external output from the parent model
		else if ((*recv_iter).model == parent)
		{
			route(parent->getParent(),parent,(*recv_iter).value);
		}
		// otherwise it is an input to a coupled model
		else
		{
			route((*recv_iter).model->typeIsNetwork(),
			(*recv_iter).model,(*recv_iter).value);
		}
	}
	recvs->clear();
	recv_pool.destroy_obj(recvs);
}

template <class X, class T>
void Simulator<X,T>::getAllChildren(Network<X,T>* model, Set<Devs<X,T>*>& s)
{
	Set<Devs<X,T>*> tmp;
	// Get the component set
	model->getComponents(tmp);
	// Add all of the local level elements to s
	s.insert(tmp.begin(),tmp.end());
	// Find the components of type network and update s recursively
	typename Set<Devs<X,T>*>::iterator iter;
	for (iter = tmp.begin(); iter != tmp.end(); iter++)
	{
		if ((*iter)->typeIsNetwork() != NULL)
		{
			getAllChildren((*iter)->typeIsNetwork(),s);
		}
	}
}

template <class X, class T>
Simulator<X,T>::~Simulator()
{
	// Clean up the models with stale IO
	typename Bag<Atomic<X,T>*>::iterator iter;
	for (iter = activated.begin(); iter != activated.end(); iter++)
	{
		clean_up(*iter);
	}
}

} // End of namespace

#endif
