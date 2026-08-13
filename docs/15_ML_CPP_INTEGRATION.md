# 15 — ML Integration into C++

## Runtime flow
```text
Approved Python Model
↓
Export
↓
C++ ModelPredictor
↓
fraud_probability
↓
RiskEngine
```

Use `IModelPredictor` to isolate the runtime.

Possible runtime technologies can include ONNX or another suitable approach, but first document:
- compatibility
- build complexity
- model support
- inference latency
- deployment implications

## ML failure policy
Fail-open vs fail-closed is a human decision.

The system must define behavior for:
- model unavailable
- timeout
- invalid model output
- feature mismatch
- model version mismatch

Never silently bypass risk controls.
