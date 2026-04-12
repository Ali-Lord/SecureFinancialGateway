-- Sample data (for the sake of demo)
-- NOTE: password_hash is placeholder to be replace with Argon2 hash.
--       Generate password hash with backend/utils/GenerateHash I created.
INSERT INTO users (email, password_hash, role) VALUES
('admin@gateway.local', '$2a$12$placeholderhashforadmin1234567890', 'admin'),
('user1@gateway.local', '$2a$12$placeholderhashforuser11234567890', 'user'),
('merchant1@gateway.local', '$2a$12$placeholderhashformerchant123456', 'user')
ON CONFLICT (email) DO NOTHING;

INSERT INTO transactions (txn_id, user_id, amount, currency, status, processor_ref, metadata) VALUES
('txn_20260410_001', 1,  1250.75, 'HKD', 'success', 'proc_stripe_abc123', '{"method": "visa", "ip": "127.0.0.1"}'),
('txn_20260410_002', 2,   450.00, 'HKD', 'success', 'proc_paypal_def456', '{"method": "paypal", "ip": "127.0.0.1"}'),
('txn_20260410_003', 1,  2890.50, 'HKD', 'pending',  'proc_stripe_ghi789', '{"method": "mastercard", "ip": "127.0.0.1"}'),
('txn_20260410_004', 3,   175.25, 'HKD', 'success', 'proc_stripe_jkl012', '{"method": "visa", "ip": "127.0.0.1"}'),
('txn_20260409_005', 2,  9200.00, 'HKD', 'failed',  'proc_paypal_mno345', '{"method": "paypal", "ip": "127.0.0.1", "reason": "insufficient_funds"}'),
('txn_20260409_006', 1,   340.80, 'HKD', 'success', 'proc_stripe_pqr678', '{"method": "mastercard", "ip": "127.0.0.1"}'),
('txn_20260408_007', 3, 12500.00, 'HKD', 'success', 'proc_stripe_stu901', '{"method": "visa", "ip": "127.0.0.1"}'),
('txn_20260408_008', 2,    89.99, 'HKD', 'refunded', 'proc_stripe_vwx234', '{"method": "visa", "ip": "127.0.0.1"}'),
('txn_20260410_009', 1,   675.40, 'HKD', 'success', 'proc_paypal_yza567', '{"method": "paypal", "ip": "127.0.0.1"}'),
('txn_20260407_010', 3,  4500.00, 'HKD', 'pending',  'proc_stripe_bcd890', '{"method": "mastercard", "ip": "127.0.0.1"}')
ON CONFLICT (txn_id) DO NOTHING;

INSERT INTO audit_log (user_id, action, ip_address, details) VALUES
(1, 'login_success', '127.0.0.1', '{"browser": "chrome", "success": true}'),
(2, 'login_success', '127.0.0.1', '{"browser": "firefox", "success": true}'),
(1, 'transaction_create', '127.0.0.1', '{"txn_id": "txn_20260410_001", "amount": 1250.75}'),
(3, 'transaction_create', '127.0.0.1', '{"txn_id": "txn_20260410_004", "amount": 175.25}'),
(2, 'transaction_failed', '127.0.0.1', '{"txn_id": "txn_20260409_005", "reason": "insufficient_funds"}'),
(1, 'dashboard_view', '127.0.0.1', '{"page": "overview"}'),
(1, 'login_success', '127.0.0.1', '{"browser": "chrome", "success": true}')
ON CONFLICT DO NOTHING;

UPDATE transactions
SET created_at = NOW() - INTERVAL '1 day' * (random() * 10)::int,
    updated_at = created_at + INTERVAL '1 hour' * (random() * 5)::int;
