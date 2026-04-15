import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'

export default defineConfig({
  server: {
    host: true,
    proxy: {
      '/api': {
        target: 'https://localhost:8443', // backend port
        changeOrigin: true,
        secure: false, // had to set it false cuz I'm using self-signed certificate (for demo purposes)
      },
    },
  },
  plugins: [
	react(),
	tailwindcss()
  ],
})
