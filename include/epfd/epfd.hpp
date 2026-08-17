#ifndef EPFD_EPFD_HPP
#define EPFD_EPFD_HPP

/**
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Assessment System
 * Master umbrella header
 */

#include "epfd/common/Types.hpp"
#include "epfd/common/Constants.hpp"

// Domain Models
#include "epfd/models/Location.hpp"
#include "epfd/models/Device.hpp"
#include "epfd/models/PaymentMethod.hpp"
#include "epfd/models/Merchant.hpp"
#include "epfd/models/Account.hpp"
#include "epfd/models/Customer.hpp"
#include "epfd/models/FraudAlert.hpp"
#include "epfd/models/RiskAssessment.hpp"
#include "epfd/models/Dispute.hpp"
#include "epfd/models/Transaction.hpp"

// OOP Interfaces & Patterns (SOLID)
#include "epfd/fraud/IFraudRule.hpp"
#include "epfd/fraud/IFraudDetector.hpp"
#include "epfd/ml/IModelPredictor.hpp"
#include "epfd/ml/MockModelPredictor.hpp"
#include "epfd/risk/IRiskRule.hpp"
#include "epfd/risk/IRiskPolicy.hpp"
#include "epfd/decision/IDecisionPolicy.hpp"
#include "epfd/database/IRepository.hpp"
#include "epfd/database/ITransactionRepository.hpp"
#include "epfd/database/ICustomerRepository.hpp"
#include "epfd/database/InMemoryRepositories.hpp"
#include "epfd/utils/Observer.hpp"

#endif // EPFD_EPFD_HPP
