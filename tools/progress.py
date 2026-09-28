"""Report matching progress: game-code bytes matched / total game-code bytes."""
import json, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
funcs = [l.rstrip('\n').split('\t') for l in open(os.path.join(ROOT, 'build/funcs.tsv'))]
game = {int(s, 16): int(n) for s, e, n, name, kind in funcs if kind == 'game'}
import glob
res = {}
for f in glob.glob(os.path.join(ROOT, 'build/match/*.json')):
    res.update(json.load(open(f)))
done = {int(a, 16) for a, r in res.items() if r['ok'] and r.get('src', '').startswith('src_match/')}
mb = sum(n for a, n in game.items() if a in done)
tb = sum(game.values())
print('game functions matched: %d / %d' % (len(done & set(game)), len(game)))
print('game bytes matched:     %d / %d (%.2f%%)' % (mb, tb, 100.0 * mb / tb))
