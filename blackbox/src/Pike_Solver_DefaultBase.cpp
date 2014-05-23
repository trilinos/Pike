#include "Pike_Solver_DefaultBase.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_VerboseObjectParameterListHelpers.hpp"
#include "Pike_BlackBoxModelEvaluator.hpp"
#include "Pike_DataTransfer.hpp"
#include "Pike_SolverObserver.hpp"
#include "Teuchos_Assert.hpp"
#include <sstream>

namespace pike {

  SolverDefaultBase::SolverDefaultBase() :
    numberOfIterations_(0),
    status_(pike::UNCHECKED),
    registrationComplete_(false)
  {
    validParameters_ = Teuchos::parameterList();
    validParameters_->set("Print Begin Solve Status",true, "If set to true the status tests will print current status at the beginning of the solve.");
    validParameters_->set("Print Step Status",true, "If set to true the status tests will print current status at the end of each step.");
    validParameters_->set("Print End Solve Status",true,"If set to true the status tests will print current status at the end of the solve.");
    validParameters_->set("Name","","A unique identifier chosen by the user for this solver. Used mainly for distinguishing nodes in a hierarchic problem.");
    Teuchos::setupVerboseObjectSublist(validParameters_.get());
  }

  SolverDefaultBase::~SolverDefaultBase() {}

  void SolverDefaultBase::registerComm(const Teuchos::RCP<const Teuchos::Comm<int> >&)
  {  
    // Default impl does nothing.  If a solver can support use of a
    // comm, it must be implemented in the base class
  }

  void SolverDefaultBase::registerModelEvaluator(const Teuchos::RCP<pike::BlackBoxModelEvaluator>& me)
  {
    TEUCHOS_TEST_FOR_EXCEPTION(registrationComplete_,
			       std::logic_error,
			       "Can NOT register model evaluators after registrationComplete() has been called!");
    models_.push_back(me);
  }
  
  void SolverDefaultBase::registerDataTransfer(const Teuchos::RCP<pike::DataTransfer>& dt)
  {
    TEUCHOS_TEST_FOR_EXCEPTION(registrationComplete_,
			       std::logic_error,
			       "Can NOT register data transfers after registrationComplete() has been called!");
    transfers_.push_back(dt);
  }
  
  void SolverDefaultBase::completeRegistration()
  {
    // Set the defaults so the user doesn't have to set the parameter list
    if (is_null(this->getMyParamList())) {
      Teuchos::RCP<Teuchos::ParameterList> defaultParameters = Teuchos::parameterList();
      this->setParameterList(defaultParameters);
    }

    registrationComplete_ = true;
  }

  Teuchos::RCP<const pike::BlackBoxModelEvaluator> SolverDefaultBase::getModelEvaluator(const std::string& meName) const
  {
    for (ModelConstIterator m = models_.begin(); m != models_.end(); ++m)
      if ((*m)->name() == meName)
	return *m;

    std::ostringstream os;
    for (ModelConstIterator m = models_.begin(); m != models_.end(); ++m)
       os << "  " << (*m)->name() << std::endl;

    TEUCHOS_TEST_FOR_EXCEPTION(true,std::logic_error,"Failed to find the ModelEvaluator named \"" << meName << "\" in the solver.  Valid models are:\n" << os.str() << std::endl);
    return Teuchos::null;
  }

  const std::vector<Teuchos::RCP<const pike::BlackBoxModelEvaluator> > SolverDefaultBase::getModelEvaluators() const
  {
    std::vector<Teuchos::RCP<const pike::BlackBoxModelEvaluator> > constModels(models_.size());
    std::copy(models_.begin(),models_.end(),constModels.begin());
    return constModels;
  }

  Teuchos::RCP<const pike::DataTransfer> SolverDefaultBase::getDataTransfer(const std::string& dtName) const
  {
    for (TransferConstIterator t = transfers_.begin(); t != transfers_.end(); ++t)
      if ((*t)->name() == dtName)
	return *t;

    TEUCHOS_TEST_FOR_EXCEPTION(true,std::logic_error,"Failed to find the DataTransfer named \"" << dtName << "\" in the solver.");
    return Teuchos::null;
  }

  const std::vector<Teuchos::RCP<const pike::DataTransfer> > SolverDefaultBase::getDataTransfers() const
  { std::vector<Teuchos::RCP<const pike::DataTransfer> > constTransfers;
    std::copy(transfers_.begin(),transfers_.end(),constTransfers.begin());
    return constTransfers;
  }

  pike::SolveStatus SolverDefaultBase::step()
  {
    for (ObserverIterator observer = observers_.begin(); observer != observers_.end(); ++observer)
      (*observer)->observeBeginStep(*this);

    this->stepImplementation();
    ++numberOfIterations_;

    status_ = statusTests_->checkStatus(*this);
    
    if (printStepStatus_) {
      Teuchos::RCP<Teuchos::FancyOStream> os = this->getOStream();
      *os << "\n** ";
      if (name_ != "")
	*os << name_ << ": ";
      *os << "Step " << this->getNumberOfIterations() 
	  << " Status **" << std::endl;
      os->pushTab(defaultIndentation);
      *os << *statusTests_;
      os->popTab();
    }

    for (ObserverIterator observer = observers_.begin(); observer != observers_.end(); ++observer)
      (*observer)->observeEndStep(*this);

    return status_;
  }
  
  pike::SolveStatus SolverDefaultBase::solve()
  {
    TEUCHOS_ASSERT(registrationComplete_);

    for (ObserverIterator observer = observers_.begin(); observer != observers_.end(); ++observer)
      (*observer)->observeBeginSolve(*this);

    status_ = statusTests_->checkStatus(*this);
    
    if (printBeginSolveStatus_) {
      Teuchos::RCP<Teuchos::FancyOStream> os = this->getOStream();
      *os << "\n** ";
      if (name_ != "")
	*os << name_ << ": ";
      *os << "Begin Solve Status **" << std::endl;
      os->pushTab(defaultIndentation);
      *os << *statusTests_;
      os->popTab();
    }

    while ( (status_ != CONVERGED) && (status_ != FAILED) )
      this->step();
    
    if (printEndSolveStatus_) {
      Teuchos::RCP<Teuchos::FancyOStream> os = this->getOStream();
      *os << "\n** ";
      if (name_ != "")
	*os << name_ << ": ";
      *os << "End Solve Status **" << std::endl;
      os->pushTab(defaultIndentation);
      *os << *statusTests_;
      os->popTab();
    }

    for (ObserverIterator observer = observers_.begin(); observer != observers_.end(); ++observer)
      (*observer)->observeEndSolve(*this);

    if (status_ == CONVERGED)
      for (ObserverIterator observer = observers_.begin(); observer != observers_.end(); ++observer)
	(*observer)->observeConvergedSolve(*this);

    if (status_ == FAILED)
      for (ObserverIterator observer = observers_.begin(); observer != observers_.end(); ++observer)
	(*observer)->observeFailedSolve(*this);

    return status_;
  }

  void SolverDefaultBase::reset()
  {
    numberOfIterations_ = 0;
    status_ = pike::UNCHECKED;
    statusTests_->reset();
  }
  
  pike::SolveStatus SolverDefaultBase::getStatus() const
  { return status_; }

  int SolverDefaultBase::getNumberOfIterations() const
  { return numberOfIterations_; }
  
  void SolverDefaultBase::setParameterList(const Teuchos::RCP<Teuchos::ParameterList>& paramList)
  {
    paramList->validateParametersAndSetDefaults(*(this->getValidParameters()));
    printBeginSolveStatus_ = paramList->get<bool>("Print Begin Solve Status");
    printStepStatus_ = paramList->get<bool>("Print Step Status");
    printEndSolveStatus_ = paramList->get<bool>("Print End Solve Status");
    name_ = paramList->get<std::string>("Name");
    this->setMyParamList(paramList);
  }
  
  Teuchos::RCP<const Teuchos::ParameterList> SolverDefaultBase::getValidParameters() const
  { return validParameters_; }

  Teuchos::RCP<Teuchos::ParameterList> SolverDefaultBase::getNonconstValidParameters()
  { return validParameters_; }

  void SolverDefaultBase::addObserver(const Teuchos::RCP<pike::SolverObserver>& observer)
  {
    observers_.push_back(observer);
  }

  std::vector<Teuchos::RCP<pike::SolverObserver> > SolverDefaultBase::getObservers() const
  {
    return observers_;
  }

  void SolverDefaultBase::setStatusTests(const Teuchos::RCP<pike::StatusTest>& statusTests)
  {
    statusTests_ = statusTests;
  }

  Teuchos::RCP<const pike::StatusTest> SolverDefaultBase::getStatusTests() const
  {
    return statusTests_;
  }
  
  std::string SolverDefaultBase::name() const
  {
    return name_;
  }
}
