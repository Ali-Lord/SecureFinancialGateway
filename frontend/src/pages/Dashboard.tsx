import {useEffect, useState} from 'react';
import api from '../services/api';
import {useNavigate} from 'react-router-dom';

interface Transaction {
  txn_id: string;
  amount: number;
  currency: string;
  status: string;
  processor_ref: string;
  created_at: string;
}

interface DashboardData {
  total_volume_hkd: number;
  success_rate_pct: number;
  total_transactions: number;
  recent_transactions: Transaction[];
}

export default function Dashboard() {
  const [data, setData] = useState<DashboardData | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const fetchDashboard = async () => {
    try {
      const res = await api.get('/status');
      setData(res.data);
    } catch (err: any) {
      setError('Failed too load dashboard');
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchDashboard();
  }, []);

  const handleLogout = () => {
    localStorage.removeItem('token'); // TODO: use httpOnly cookie
    navigate('/login', {replace: true});
  };

  if (loading) {
    return (
      <div className="min-h-screen bg-gray-950 flex items-center justify-center">
        <p className="text-gray-400">Loading dashboard...</p>
      </div>
    );
  }

  if (error || !data) {
    return (
      <div className="min-h-screen bg-gray-950 flex items-center justify-center">
        <p className="text-red-500">{error || 'Failed to load data'}</p>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-[#f4f4f4] text-[#1a1a1a] font-sans">
      {/* Top red bar */}
      <div
        style={{ backgroundColor: '#db0011' }}
        className="h-1.5 w-full fixed top-0 z-[60]"
      />

      <nav className="bg-white border-b border-gray-200 px-8 py-4 flex justify-between items-center sticky top-[6px] z-50 shadow-sm">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <h1 className="text-xl font-bold tracking-tight">
              Secure Financial <span className="text-[#db0011]">Gateway</span>
            </h1>
          </div>
        </div>

        <div className="flex items-center gap-4">
          <span className="text-[10px] font-mono text-gray-400 uppercase">Payment Card Industry Data Security Standard (PCI DSS)</span>
          <button
            onClick={handleLogout}
            className="border border-[#1a1a1a] px-4 py-1.5 text-xs font-bold hover:bg-[#1a1a1a] hover:text-white transition-all"
          >
            LOGOUT
          </button>
        </div>
      </nav>

      {/* TODO: turn them into components */}
      <main className="p-10 max-w-7xl mx-auto">
        <div className="grid grid-cols-1 md:grid-cols-3 gap-8 mb-12">
          {/* Metric Card */}
          <div className="bg-white border-t-4 border-t-[#db0011] p-8 shadow-sm">
            <p className="text-[10px] font-bold text-gray-400 uppercase tracking-widest mb-2">Total Volume</p>
            <p className="text-4xl font-mono font-medium">
              {data.total_volume_hkd.toLocaleString()} <span className="text-sm">HKD</span>
            </p>
          </div>

          <div className="bg-white border-t-4 border-t-gray-800 p-8 shadow-sm">
            <p className="text-[10px] font-bold text-gray-400 uppercase tracking-widest mb-2">Success Rate</p>
            <p className="text-4xl font-mono font-medium text-green-600">
              {data.success_rate_pct.toFixed(1)}%
            </p>
          </div>

          <div className="bg-white border-t-4 border-t-gray-800 p-8 shadow-sm">
            <p className="text-[10px] font-bold text-gray-400 uppercase tracking-widest mb-2">Active Nodes</p>
            <p className="text-4xl font-mono font-medium">
              {data.total_transactions.toLocaleString()}
            </p>
          </div>
        </div>

        {/* Transaction Table */}
        <div className="bg-white shadow-sm border border-gray-200">
          <div className="bg-gray-50 border-b border-gray-200 px-6 py-4">
            <h3 className="text-xs font-bold uppercase tracking-widest">Transactions</h3>
          </div>
          <table className="w-full text-left">
            <thead>
              <tr className="border-b border-gray-100 text-[10px] uppercase text-gray-400 tracking-widest">
                <th className="p-6">Reference ID</th>
                <th className="p-6">Amount</th>
                <th className="p-6">Status</th>
              </tr>
            </thead>
            <tbody className="text-sm">
              {data.recent_transactions.map((txn) => (
                <tr key={txn.txn_id} className="border-b border-gray-50 last:border-0">
                  <td className="p-6 font-mono text-xs text-[#db0011]">{txn.txn_id}</td>
                  <td className="p-6 font-bold">{txn.amount.toLocaleString()} {txn.currency}</td>
                  <td className="p-6">
                    <span className={`text-[10px] font-bold px-2 py-0.5 border ${
                      txn.status === 'success' ? 'border-green-500 text-green-500' : 'border-red-500 text-red-500'
                    }`}>
                      {txn.status.toUpperCase()}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </main>
    </div>
  );

}
