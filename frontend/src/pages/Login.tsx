import {useState} from 'react';
import {useNavigate} from 'react-router-dom';
import api from '../services/api';

export default function Login() {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);
    setError('');

    try {
      const response = await api.post('/login', {email, password});

      // TODO: switch to httpOnly cookie
      localStorage.setItem('token', response.data.token);
      navigate('/dashboard', {replace: true});
    } catch (err: any) {
      setError(err.response?.data?.error || 'Invalid email or password.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen bg-[#f4f4f4] flex flex-col items-center justify-center px-4 font-sans">
      {/* Red Top Bar */}
      <div style={{ backgroundColor: '#db0011' }} className="h-1.5 w-full fixed top-0" />

      <div className="w-full max-w-[400px] bg-white p-10 shadow-xl border border-gray-200">
        {/* Header */}
        <div className="mb-10 text-center">
          <div className="flex justify-center mb-6">
            {/* Logo */}
            <div className="w-10 h-10 bg-[#db0011] flex items-center justify-center">
              <span className="text-white font-bold text-xs tracking-tighter">SFG</span>
            </div>
          </div>
          <h1 className="text-2xl font-bold tracking-tight text-[#1a1a1a] mb-2 uppercase">
            Secure Financial <span className="text-[#db0011]">Gateway</span>
          </h1>
          <p className="text-gray-400 text-[10px] font-bold uppercase tracking-[0.2em]">
            Cybersecurity Demo
          </p>
        </div>

        <form onSubmit={handleSubmit} className="space-y-6">
          <div className="space-y-4">
            {/* TODO: make use of components */}
            {/* Email field */}
            <div className="group">
              <label className="block text-[10px] font-bold text-gray-500 uppercase tracking-widest mb-1 group-focus-within:text-[#db0011] transition-colors">
                Email Address
              </label>
              <input
                type="email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                className="w-full bg-gray-50 border border-gray-200 px-4 py-3 text-sm text-[#1a1a1a] focus:outline-none focus:border-[#db0011] transition-all placeholder:text-gray-300"
                placeholder="email@address.com"
                required
              />
            </div>

            {/* Password field */}
            <div className="group">
              <label className="block text-[10px] font-bold text-gray-500 uppercase tracking-widest mb-1 group-focus-within:text-[#db0011] transition-colors">
                Password
              </label>
              <input
                type="password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                className="w-full bg-gray-50 border border-gray-200 px-4 py-3 text-sm text-[#1a1a1a] focus:outline-none focus:border-[#db0011] transition-all"
                required
              />
            </div>
          </div>

          {/* Login button */}
          <button
            type="submit"
            disabled={loading}
            className="w-full bg-[#1a1a1a] text-white text-xs font-bold py-4 hover:bg-[#db0011] active:scale-[0.98] transition-all duration-300 uppercase tracking-[0.2em] shadow-lg"
          >
            {loading ? 'Verifying...' : 'Login'}
          </button>
        </form>

        {/* Error message */}
        {error && (
          <div className="mt-6 p-3 border border-[#db0011]/20 bg-[#db0011]/5">
            <p className="text-[#db0011] text-[10px] font-bold text-center uppercase tracking-tight">
              System Alert: {error}
            </p>
          </div>
        )}

        {/* Footer */}
        <footer className="mt-12 pt-6 border-t border-gray-100">
          <div className="flex justify-between items-center text-[9px] text-gray-400 font-mono uppercase tracking-widest">
            <span>Payment Card Industry <br/> Data Security Standard</span>
            <span>&copy; 2026 Ali Raz</span>
          </div>
        </footer>
      </div>
    <p className="mt-8 text-[10px] text-left text-gray-400 tracking-[0.3em] font-medium">
      https://github.com/Ali-Lord/SecureFinancialGateway
    </p>

    </div>
  );

}
