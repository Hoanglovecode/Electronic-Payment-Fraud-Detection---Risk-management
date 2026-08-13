# 08 — Rule-Based Fraud Detection

Fraud Detection asks:
> Is this transaction suspicious?

## Rule abstraction
Each rule should have:
- name
- evaluation logic
- evidence/reason
- risk/fraud contribution
- independent tests

## Initial rules
- LargeAmountRule
- HighVelocityRule
- NewDeviceRule
- NewLocationRule
- ForeignCountryRule
- UnusualTimeRule
- SuspiciousMerchantRule
- BehaviorDeviationRule

## Advanced rules
- CardTestingRule
- AccountTakeoverSignalRule
- BlacklistRule
- WhitelistRule

Clarify the data source for merchant risk; do not leave SuspiciousMerchantRule as empty logic.

Keep fraud detection separate from final risk/decision policy.
