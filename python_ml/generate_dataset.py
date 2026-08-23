import numpy as np
import pandas as pd
import os

def normalize_p(p_list):
    arr = np.array(p_list, dtype=np.float64)
    return arr / np.sum(arr)

def generate_fraud_dataset(n_samples=10000, fraud_ratio=0.025, random_state=42):
    np.random.seed(random_state)
    n_fraud = int(n_samples * fraud_ratio)
    n_legit = n_samples - n_fraud

    print(f"Generating synthetic dataset: {n_samples} total ({n_legit} legitimate, {n_fraud} fraud, {fraud_ratio*100:.1f}% fraud rate)")

    # 1. Legitimate Transactions
    legit_amount = np.random.exponential(scale=75.0, size=n_legit) + 5.0
    p_legit_hour = normalize_p([
        0.01, 0.01, 0.01, 0.01, 0.02, 0.03, 0.05, 0.06,
        0.07, 0.08, 0.08, 0.08, 0.08, 0.07, 0.07, 0.06,
        0.06, 0.05, 0.04, 0.03, 0.02, 0.02, 0.01, 0.01
    ])
    legit_hour = np.random.choice(range(24), size=n_legit, p=p_legit_hour)
    legit_dow = np.random.choice(range(7), size=n_legit)
    legit_is_weekend = (legit_dow >= 5).astype(float)
    legit_tx_5m = np.random.poisson(lam=0.05, size=n_legit)
    legit_sum_5m = legit_tx_5m * legit_amount
    legit_tx_1h = legit_tx_5m + np.random.poisson(lam=0.25, size=n_legit)
    legit_sum_1h = legit_tx_1h * legit_amount
    legit_tx_24h = legit_tx_1h + np.random.poisson(lam=1.8, size=n_legit)
    legit_sum_24h = legit_amount * (1.0 + np.random.uniform(0.1, 0.8, size=n_legit))
    legit_avg_24h = legit_sum_24h / (legit_tx_24h + 1.0)
    legit_dev_ratio = np.random.normal(loc=1.0, scale=0.2, size=n_legit).clip(0.3, 2.5)
    legit_is_new_dev = np.random.choice([0.0, 1.0], size=n_legit, p=[0.90, 0.10])
    legit_is_high_risk_dev = np.random.choice([0.0, 1.0], size=n_legit, p=[0.99, 0.01])
    legit_dev_count = np.random.poisson(lam=1.2, size=n_legit).clip(1, 3)
    legit_is_new_country = np.random.choice([0.0, 1.0], size=n_legit, p=[0.96, 0.04])
    legit_distance = np.random.exponential(scale=15.0, size=n_legit)
    legit_speed = (legit_distance / np.random.uniform(1.0, 12.0, size=n_legit)).clip(0.0, 110.0)
    legit_is_high_risk_mcc = np.random.choice([0.0, 1.0], size=n_legit, p=[0.94, 0.06])

    df_legit = pd.DataFrame({
        'transaction_amount': legit_amount,
        'hour_of_day': legit_hour,
        'is_weekend': legit_is_weekend,
        'transactions_last_5min': legit_tx_5m,
        'amount_sum_last_5min': legit_sum_5m,
        'transactions_last_1hour': legit_tx_1h,
        'amount_sum_last_1hour': legit_sum_1h,
        'transactions_last_24hours': legit_tx_24h,
        'amount_sum_last_24hours': legit_sum_24h,
        'average_amount_24h': legit_avg_24h,
        'amount_deviation_ratio': legit_dev_ratio,
        'is_new_device': legit_is_new_dev,
        'is_high_risk_device': legit_is_high_risk_dev,
        'accounts_on_device_count': legit_dev_count,
        'is_new_country': legit_is_new_country,
        'distance_from_home_km': legit_distance,
        'speed_from_last_tx_kmh': legit_speed,
        'is_high_risk_mcc': legit_is_high_risk_mcc,
        'is_fraud': 0
    })

    # 2. Fraud Transactions
    p_fraud_types = normalize_p([0.3, 0.25, 0.45])
    fraud_type_choices = np.random.choice([0, 1, 2], size=n_fraud, p=p_fraud_types)
    fraud_amount = np.zeros(n_fraud)
    for i, choice in enumerate(fraud_type_choices):
        if choice == 0:
            fraud_amount[i] = np.random.uniform(1.0, 4.9) # Micro card testing
        elif choice == 1:
            fraud_amount[i] = np.random.uniform(8200.0, 9900.0) # AML Smurfing
        else:
            fraud_amount[i] = np.random.exponential(scale=800.0) + 200.0 # Large purchase

    p_fraud_hour = normalize_p([
        0.08, 0.08, 0.09, 0.09, 0.07, 0.05, 0.03, 0.02,
        0.02, 0.02, 0.03, 0.03, 0.03, 0.04, 0.04, 0.04,
        0.05, 0.05, 0.05, 0.05, 0.04, 0.04, 0.03, 0.03
    ])
    fraud_hour = np.random.choice(range(24), size=n_fraud, p=p_fraud_hour)
    fraud_dow = np.random.choice(range(7), size=n_fraud)
    fraud_is_weekend = (fraud_dow >= 5).astype(float)
    fraud_tx_5m = np.random.poisson(lam=3.5, size=n_fraud).clip(1, 8)
    fraud_sum_5m = fraud_amount * fraud_tx_5m
    fraud_tx_1h = fraud_tx_5m + np.random.poisson(lam=4.0, size=n_fraud)
    fraud_sum_1h = fraud_amount * fraud_tx_1h
    fraud_tx_24h = fraud_tx_1h + np.random.poisson(lam=6.0, size=n_fraud)
    fraud_sum_24h = fraud_amount * np.random.uniform(2.5, 8.0, size=n_fraud)
    fraud_avg_24h = np.random.uniform(20.0, 80.0, size=n_fraud)
    fraud_dev_ratio = np.random.uniform(4.0, 15.0, size=n_fraud)
    fraud_is_new_dev = np.random.choice([0.0, 1.0], size=n_fraud, p=[0.20, 0.80])
    fraud_is_high_risk_dev = np.random.choice([0.0, 1.0], size=n_fraud, p=[0.30, 0.70])
    fraud_dev_count = np.random.choice([3, 4, 5, 6], size=n_fraud)
    fraud_is_new_country = np.random.choice([0.0, 1.0], size=n_fraud, p=[0.30, 0.70])
    fraud_distance = np.random.uniform(500.0, 8000.0, size=n_fraud)
    fraud_speed = np.random.uniform(400.0, 2500.0, size=n_fraud)
    fraud_is_high_risk_mcc = np.random.choice([0.0, 1.0], size=n_fraud, p=[0.35, 0.65])

    df_fraud = pd.DataFrame({
        'transaction_amount': fraud_amount,
        'hour_of_day': fraud_hour,
        'is_weekend': fraud_is_weekend,
        'transactions_last_5min': fraud_tx_5m,
        'amount_sum_last_5min': fraud_sum_5m,
        'transactions_last_1hour': fraud_tx_1h,
        'amount_sum_last_1hour': fraud_sum_1h,
        'transactions_last_24hours': fraud_tx_24h,
        'amount_sum_last_24hours': fraud_sum_24h,
        'average_amount_24h': fraud_avg_24h,
        'amount_deviation_ratio': fraud_dev_ratio,
        'is_new_device': fraud_is_new_dev,
        'is_high_risk_device': fraud_is_high_risk_dev,
        'accounts_on_device_count': fraud_dev_count,
        'is_new_country': fraud_is_new_country,
        'distance_from_home_km': fraud_distance,
        'speed_from_last_tx_kmh': fraud_speed,
        'is_high_risk_mcc': fraud_is_high_risk_mcc,
        'is_fraud': 1
    })

    df = pd.concat([df_legit, df_fraud], ignore_index=True)
    df = df.sample(frac=1.0, random_state=random_state).reset_index(drop=True)

    os.makedirs('data', exist_ok=True)
    output_path = 'data/fraud_transactions_dataset.csv'
    df.to_csv(output_path, index=False)
    print(f"Successfully saved dataset to {output_path}")
    return df

if __name__ == '__main__':
    generate_fraud_dataset()
