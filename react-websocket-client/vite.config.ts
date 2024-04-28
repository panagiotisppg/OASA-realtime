import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  build: {
    emptyOutDir: true,
    outDir: '../data/www',
  rollupOptions: {
        output : {
          chunkFileNames: "[name].js",
          assetFileNames: "[name][extname]",
          entryFileNames: "[name].js"
        }
    }
  }
})
