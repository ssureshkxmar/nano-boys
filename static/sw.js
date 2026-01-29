self.addEventListener('install', (e) => {
  console.log('[PWA] Installed');
});

self.addEventListener('fetch', (e) => {
  // Static pass-through for now
  e.respondWith(fetch(e.request));
});
