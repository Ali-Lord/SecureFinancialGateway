-- Sample data (based on my knowledge on transactions with PayPal)
INSERT INTO users (email, role) VALUES
('admin@gateway.local', 'admin'),
('user1@gateway.local', 'user'),
('merchant1@gateway.local', 'user')
ON CONFLICT (email) DO NOTHING;

INSERT INTO transactions (txn_id, user_id, amount, currency, status, processor_ref, metadata) VALUES
('txn_20260403_001', 1, 1250.75, 'HKD', 'success', 'proc_stripe_abc123', '{"method": "card", "ip": "127.0.0.1"}'),
('txn_20260403_002', 2, 450.00,  'HKD', 'success', 'proc_paypal_def456', '{"method": "paypal", "ip": "127.0.0.1"}'),
('txn_20260403_003', 1, 2890.50, 'HKD', 'pending',  'proc_stripe_ghi789', '{"method": "card", "ip": "127.0.0.1"}'),
('txn_20260403_004', 3, 175.25,  'HKD', 'success', 'proc_stripe_jkl012', '{"method": "card", "ip": "127.0.0.1"}'),
('txn_20260403_005', 2, 9200.00, 'HKD', 'failed',  'proc_paypal_mno345', '{"method": "paypal", "ip": "127.0.0.1", "reason": "insufficient_funds"}'),
('txn_20260402_006', 1, 340.80,  'HKD', 'success', 'proc_stripe_pqr678', '{"method": "card", "ip": "127.0.0.1"}'),
('txn_20260402_007', 3, 12500.00,'HKD', 'success', 'proc_stripe_stu901', '{"method": "card", "ip": "127.0.0.1"}'),
('txn_20260401_008', 2, 89.99,   'HKD', 'refunded','proc_stripe_vwx234', '{"method": "card", "ip": "127.0.0.1"}'),
('txn_20260403_009', 1, 675.40,  'HKD', 'success', 'proc_paypal_yza567', '{"method": "paypal", "ip": "127.0.0.1"}'),
('txn_20260403_010', 3, 4500.00, 'HKD', 'pending',  'proc_stripe_bcd890', '{"method": "card", "ip": "127.0.0.1"}')
ON CONFLICT (txn_id) DO NOTHING;

INSERT INTO audit_log (user_id, action, ip_address, details) VALUES
(1, 'login', '127.0.0.1', '{"browser": "chrome", "success": true}'),
(2, 'transaction_create', '127.0.0.1', '{"txn_id": "txn_20260403_001", "amount": 1250.75}'),
(1, 'transaction_create', '127.0.0.1', '{"txn_id": "txn_20260403_003", "amount": 2890.50}'),
(3, 'login', '127.0.0.1', '{"browser": "firefox", "success": true}'),
(2, 'transaction_failed', '127.0.0.1', '{"txn_id": "txn_20260403_005", "reason": "insufficient_funds"}'),
(1, 'dashboard_view', '127.0.0.1', '{"page": "overview"}')
ON CONFLICT DO NOTHING;

UPDATE transactions 
SET created_at = NOW() - INTERVAL '1 day' * floor(random() * 5),
    updated_at = created_at + INTERVAL '1 hour' * floor(random() * 3)
WHERE created_at = updated_at;
