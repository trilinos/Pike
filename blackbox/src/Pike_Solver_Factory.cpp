#include "Pike_Solver_Factory.hpp"
#include "Teuchos_Assert.hpp"

// Pike solvers
#include "Pike_Solver_BlockGaussSeidel.hpp"
#include "Pike_Solver_BlockJacobi.hpp"
#include <algorithm>

namespace pike {

  SolverFactory::SolverFactory()
  {
    supportedTypes_.push_back("Block Gauss Seidel");
    supportedTypes_.push_back("Block Jacobi");
  }

  void SolverFactory::addFactory(const Teuchos::RCP<pike::SolverAbstractFactory>& f)
  {
    userFactories_.push_back(f);
  }
  
  bool SolverFactory::supportsType(const std::string& type) const
  {
    return (std::find(supportedTypes_.begin(),supportedTypes_.end(),type) != supportedTypes_.end());
  }

  Teuchos::RCP<pike::Solver> 
  SolverFactory::buildSolver(const Teuchos::RCP<Teuchos::ParameterList>& p) const
  {
    TEUCHOS_TEST_FOR_EXCEPTION(!p->isType<std::string>("Solver Sublist Name"), std::logic_error,
			       "The \"Solver Sublist Name\" must be specified but is missing!");
    
    std::string solverSublistName = p->get<std::string>("Solver Sublist Name");
    
    TEUCHOS_TEST_FOR_EXCEPTION(!p->isSublist(solverSublistName), std::logic_error,
			       "Error the requested \"Solver Sublist Name\" with value \"" 
			       << solverSublistName << "\" is not valid!");
    
    Teuchos::RCP<Teuchos::ParameterList> solverSublist = Teuchos::sublist(p,solverSublistName);
    
    TEUCHOS_TEST_FOR_EXCEPTION(!solverSublist->isType<std::string>("Type"),std::logic_error,
			       "The \"Type\" key must be specified for the solver sublist \"" 
			       << solverSublistName << "\"!");
    
    std::string type = solverSublist->get<std::string>("Type");
    
    Teuchos::RCP<pike::Solver> solver;

    if (type == "Block Gauss Seidel") {
      solver = Teuchos::rcp(new pike::BlockGaussSeidel);
      solver->setParameterList(solverSublist);
    }
    else if (type == "Block Jacobi") {
      solver = Teuchos::rcp(new pike::BlockJacobi);
      solver->setParameterList(solverSublist);
    }
    else {
      for (std::vector<Teuchos::RCP<pike::SolverAbstractFactory> >::const_iterator i=userFactories_.begin();
	   i != userFactories_.end(); ++i) {
	if ( (*i)->supportsType(type) )
	  solver = (*i)->buildSolver(p);
      }
    }

    TEUCHOS_TEST_FOR_EXCEPTION(is_null(solver), std::runtime_error,
			       "Error - the requested solver \"Type\" with value \""<< type 
			       << "\" is not supported by the Solver Factory!");

    return solver;
  }
  
  void validateParameterList(const Teuchos::RCP<Teuchos::ParameterList>& p)
  {
    for (Teuchos::ParameterList::ConstIterator entry = p->begin(); entry != p->end(); ++entry) {

      if ( entry->first == "Solver Sublist Name" ) {
	// OK: valid param
      }
      else if ( entry->first == "Status Test Sublist Name" )  {
	// OK: valid param
      }
      else if ( entry->second.isList() ) {
	// OK: valid param
      }
      else {
	TEUCHOS_TEST_FOR_EXCEPTION(true,std::logic_error,
				   "Error the top level Pike parameter list is not valid.  All entries in the top level must be sublists or one of the two keys: \"Solver Sublist Name\" or \"Status Test Sublist Name\"!");
      }

    }

  }
}
