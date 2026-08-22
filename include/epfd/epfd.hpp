#ifndef EPFD_EPFD_HPP
#define EPFD_EPFD_HPP

/**
 * EPFD-RAS: Electronic Payment Fraud Detection & Risk Assessment System
 * Master umbrella header
 */

#include "epfd/common/Types.hpp"
#include "epfd/common/Constants.hpp"

// Custom Data Structures Templates (Core DSA Library)
#include "epfd/dsa/Vector.hpp"
#include "epfd/dsa/Deque.hpp"
#include "epfd/dsa/LinkedList.hpp"
#include "epfd/dsa/HashMap.hpp"
#include "epfd/dsa/HashSet.hpp"
#include "epfd/dsa/PriorityQueue.hpp"
#include "epfd/dsa/Queue.hpp"

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

// Validation Layer
#include "epfd/validation/ValidationResult.hpp"
#include "epfd/validation/TransactionValidator.hpp"

// Persistence & Repositories
#include "epfd/database/IRepository.hpp"
#include "epfd/database/ITransactionRepository.hpp"
#include "epfd/database/ICustomerRepository.hpp"
#include "epfd/database/IAccountRepository.hpp"
#include "epfd/database/InMemoryRepositories.hpp"

// Data Structures & Algorithms System Components (DSA)
#include "epfd/dsa/TimeWindowBuffer.hpp"
#include "epfd/dsa/CustomerVelocityTracker.hpp"
#include "epfd/dsa/FastLookupIndex.hpp"
#include "epfd/dsa/InvestigationPriorityQueue.hpp"
#include "epfd/dsa/FraudRingGraph.hpp"
#include "epfd/dsa/RiskRankingUtils.hpp"

// Feature Engineering Layer
#include "epfd/features/TransactionFeatures.hpp"
#include "epfd/features/FeatureExtractor.hpp"

// Fraud Detection Layer (Rule-Based & Engine)
#include "epfd/fraud/IFraudRule.hpp"
#include "epfd/fraud/IFraudDetector.hpp"
#include "epfd/fraud/ConcreteFraudRules.hpp"
#include "epfd/fraud/FraudDetectorEngine.hpp"

// ML Predictor Abstractions
#include "epfd/ml/IModelPredictor.hpp"
#include "epfd/ml/MockModelPredictor.hpp"

// Risk Management Layer
#include "epfd/risk/RiskFactor.hpp"
#include "epfd/risk/RiskProfile.hpp"
#include "epfd/risk/RiskWeights.hpp"
#include "epfd/risk/RiskAggregator.hpp"
#include "epfd/risk/IRiskRule.hpp"
#include "epfd/risk/IRiskPolicy.hpp"
#include "epfd/risk/ConcreteRiskPolicies.hpp"
#include "epfd/risk/RiskEngine.hpp"

// Decision & Strategy Patterns (SOLID)
#include "epfd/decision/IDecisionPolicy.hpp"
#include "epfd/utils/Observer.hpp"

// Services Layer
#include "epfd/services/TransactionService.hpp"

#endif // EPFD_EPFD_HPP
