/**
 * Assembly guide interactions (docs/build/index.html):
 *
 *   1. Click / Enter / Space a step photo to open it full size in the <dialog>.
 *   2. Highlight the current step in the sticky step rail while scrolling.
 *
 * Both are enhancements only - the guide is complete without them. Photos are
 * plain <img> tags, the rail is plain anchor links, and a figure still showing
 * its photo brief is hidden by CSS rather than by anything here.
 *
 * Dependency-free and defensive: every hook is optional, so nothing throws if
 * the markup is absent or the browser is old.
 */

/** Only photos that actually rendered are zoomable; unshot ones show a brief. */
const ZOOMABLE = '.shot:not(.missing) img';

function wireLightbox(): void {
  const dialog = document.querySelector<HTMLDialogElement>('.lightbox');
  if (!dialog || typeof dialog.showModal !== 'function') return;

  const full = dialog.querySelector('img');
  const caption = dialog.querySelector<HTMLElement>('.lightbox-caption');
  if (!full) return;

  const open = (source: HTMLImageElement): void => {
    full.src = source.currentSrc || source.src;
    full.alt = source.alt;

    if (caption) {
      const text = source.closest('figure')?.querySelector('figcaption')?.textContent?.trim() || source.alt;
      caption.textContent = text;
      caption.hidden = text.length === 0;
    }

    dialog.showModal();
  };

  const source = (target: EventTarget | null): HTMLImageElement | null => {
    const found = (target as Element | null)?.closest?.(ZOOMABLE);
    return found instanceof HTMLImageElement ? found : null;
  };

  document.addEventListener('click', (event) => {
    const img = source(event.target);
    if (img) open(img);
  });

  // <img> is focusable (tabindex="0"), so it must answer the keyboard too.
  document.addEventListener('keydown', (event) => {
    if (event.key !== 'Enter' && event.key !== ' ') return;
    const img = source(event.target);
    if (!img) return;
    event.preventDefault();
    open(img);
  });

  // Escape is native to <dialog>; a backdrop click and the close button are not.
  dialog.addEventListener('click', (event) => {
    if (event.target === dialog) dialog.close();
  });
  dialog.querySelector('.lightbox-close')?.addEventListener('click', () => dialog.close());
}

function wireStepRail(): void {
  const rail = document.querySelector('.step-rail');
  if (!rail || typeof IntersectionObserver === 'undefined') return;

  const links = Array.from(rail.querySelectorAll<HTMLAnchorElement>('a[href^="#"]'));
  const steps = links
    .map((link) => document.getElementById(decodeURIComponent(link.hash.slice(1))))
    .filter((element): element is HTMLElement => element !== null);
  if (steps.length === 0) return;

  const highlight = (id: string | null): void => {
    for (const link of links) {
      const active = id !== null && decodeURIComponent(link.hash.slice(1)) === id;
      link.parentElement?.classList.toggle('active', active);
    }
  };

  const onScreen = new Set<string>();
  const observer = new IntersectionObserver(
    (entries) => {
      for (const entry of entries) {
        if (entry.isIntersecting) onScreen.add(entry.target.id);
        else onScreen.delete(entry.target.id);
      }
      highlight(steps.find((step) => onScreen.has(step.id))?.id ?? null);
    },
    // Count a step as "current" once it reaches the upper part of the viewport.
    { rootMargin: '-8% 0px -68% 0px' },
  );

  for (const step of steps) observer.observe(step);
}

wireLightbox();
wireStepRail();
