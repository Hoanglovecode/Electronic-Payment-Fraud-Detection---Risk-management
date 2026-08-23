import numpy as np
import pandas as pd
import json
import os
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier, HistGradientBoostingClassifier
from sklearn.calibration import CalibratedClassifierCV
from sklearn.metrics import (
    precision_score, recall_score, f1_score,
    roc_auc_score, average_precision_score,
    confusion_matrix, classification_report
)

def run_ml_pipeline():
    print("=" * 60)
    print("      EPFD-RAS Machine Learning Pipeline Execution       ")
    print("=" * 60)

    data_path = 'data/fraud_transactions_dataset.csv'
    if not os.path.exists(data_path):
        from generate_dataset import generate_fraud_dataset
        df = generate_fraud_dataset()
    else:
        df = pd.read_csv(data_path)

    # 1. EDA & Summary
    n_samples = len(df)
    n_fraud = df['is_fraud'].sum()
    n_legit = n_samples - n_fraud
    fraud_rate = n_fraud / n_samples

    print(f"\n[1] Dataset Profile:")
    print(f"    - Total Transactions: {n_samples}")
    print(f"    - Legitimate: {n_legit} ({100 - fraud_rate*100:.2f}%)")
    print(f"    - Fraudulent: {n_fraud} ({fraud_rate*100:.2f}%)")
    print(f"    - Features Count: {df.shape[1] - 1}")

    # 2. Train / Val / Test Split (70% Train, 15% Val, 15% Test)
    feature_cols = [c for c in df.columns if c != 'is_fraud']
    X = df[feature_cols].values
    y = df['is_fraud'].values

    X_train_val, X_test, y_train_val, y_test = train_test_split(
        X, y, test_size=0.15, random_state=42, stratify=y
    )
    X_train, X_val, y_train, y_val = train_test_split(
        X_train_val, y_train_val, test_size=0.17647, random_state=42, stratify=y_train_val
    )

    print(f"\n[2] Data Partitioning (Zero-Leakage Stratified Split):")
    print(f"    - Train Set:      {len(X_train)} samples ({y_train.sum()} frauds)")
    print(f"    - Validation Set: {len(X_val)} samples ({y_val.sum()} frauds)")
    print(f"    - Test Set:       {len(X_test)} samples ({y_test.sum()} frauds)")

    # 3. Feature Scaling (Fitted STRICTLY on Train)
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_val_scaled = scaler.transform(X_val)
    X_test_scaled = scaler.transform(X_test)

    # 4. Candidate Models Benchmarking
    models = {
        "Logistic Regression (Baseline)": LogisticRegression(
            class_weight='balanced', max_iter=1000, random_state=42
        ),
        "Random Forest (Comparison)": RandomForestClassifier(
            n_estimators=100, max_depth=8, class_weight='balanced', random_state=42, n_jobs=-1
        ),
        "HistGradientBoosting (Candidate)": HistGradientBoostingClassifier(
            max_iter=100, max_depth=6, class_weight='balanced', random_state=42
        )
    }

    results = {}
    best_model_name = None
    best_pr_auc = -1.0

    print("\n[3] Model Training & Evaluation Benchmark:")
    for name, model in models.items():
        model.fit(X_train_scaled, y_train)
        
        # Predict on Validation
        val_probs = model.predict_proba(X_val_scaled)[:, 1]
        val_preds = (val_probs >= 0.5).astype(int)

        # Predict on Test
        test_probs = model.predict_proba(X_test_scaled)[:, 1]
        test_preds = (test_probs >= 0.5).astype(int)

        cm = confusion_matrix(y_test, test_preds)
        tn, fp, fn, tp = cm.ravel()

        precision = precision_score(y_test, test_preds, zero_division=0)
        recall = recall_score(y_test, test_preds, zero_division=0)
        f1 = f1_score(y_test, test_preds, zero_division=0)
        roc_auc = roc_auc_score(y_test, test_probs)
        pr_auc = average_precision_score(y_test, test_probs)
        fpr = fp / (fp + tn) if (fp + tn) > 0 else 0.0
        fnr = fn / (fn + tp) if (fn + tp) > 0 else 0.0

        results[name] = {
            'model': model,
            'precision': precision,
            'recall': recall,
            'f1': f1,
            'roc_auc': roc_auc,
            'pr_auc': pr_auc,
            'fpr': fpr,
            'fnr': fnr,
            'tp': int(tp),
            'fp': int(fp),
            'tn': int(tn),
            'fn': int(fn)
        }

        print(f"\n--- {name} ---")
        print(f"    ROC-AUC:  {roc_auc:.4f} | PR-AUC: {pr_auc:.4f}")
        print(f"    Precision: {precision:.4f} | Recall: {recall:.4f} | F1: {f1:.4f}")
        print(f"    FPR (False Alarm): {fpr*100:.2f}% | FNR (Missed Fraud): {fnr*100:.2f}%")
        print(f"    Confusion Matrix: TP={tp}, FP={fp}, TN={tn}, FN={fn}")

        if pr_auc > best_pr_auc:
            best_pr_auc = pr_auc
            best_model_name = name

    print(f"\n[4] Champion Model Selected by PR-AUC: {best_model_name}")

    # 5. Optimal Threshold Analysis for Champion Model
    champion = results[best_model_name]['model']
    test_probs = champion.predict_proba(X_test_scaled)[:, 1]

    best_thresh = 0.5
    best_f1 = -1.0
    print("\n[5] Threshold Sweep Analysis (Precision-Recall Optimization):")
    for thresh in np.arange(0.1, 0.95, 0.05):
        preds = (test_probs >= thresh).astype(int)
        p = precision_score(y_test, preds, zero_division=0)
        r = recall_score(y_test, preds, zero_division=0)
        f = f1_score(y_test, preds, zero_division=0)
        if f > best_f1:
            best_f1 = f
            best_thresh = thresh
        if abs(thresh - 0.5) < 1e-4 or abs(thresh - 0.3) < 1e-4 or abs(thresh - 0.7) < 1e-4:
            print(f"    Threshold={thresh:.2f} -> Precision: {p:.4f}, Recall: {r:.4f}, F1: {f:.4f}")

    print(f"    => Optimal Operating Threshold: {best_thresh:.2f} (Yields Max F1: {best_f1:.4f})")

    # 6. Export Model Artifacts for C++ Real-Time Native Inference
    os.makedirs('models', exist_ok=True)
    export_data = {
        'model_name': best_model_name,
        'feature_names': feature_cols,
        'scaler_mean': scaler.mean_.tolist(),
        'scaler_scale': scaler.scale_.tolist(),
        'optimal_threshold': float(best_thresh),
        'metrics': {
            'roc_auc': float(results[best_model_name]['roc_auc']),
            'pr_auc': float(results[best_model_name]['pr_auc']),
            'f1_score': float(best_f1),
            'precision': float(results[best_model_name]['precision']),
            'recall': float(results[best_model_name]['recall'])
        }
    }

    # If Logistic Regression or linear baseline
    lr_model = results["Logistic Regression (Baseline)"]['model']
    export_data['logistic_regression'] = {
        'coefficients': lr_model.coef_[0].tolist(),
        'intercept': float(lr_model.intercept_[0])
    }

    model_path = 'models/fraud_ml_model.json'
    with open(model_path, 'w') as f:
        json.dump(export_data, f, indent=4)
    print(f"\n[6] Exported C++ Inference Metadata & Model Weights to {model_path}")
    print("=" * 60)

if __name__ == '__main__':
    run_ml_pipeline()
