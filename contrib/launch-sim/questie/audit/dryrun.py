import subprocess, sys, re
REMOTE = 'podman exec -i forever-db-1 sh -c "mysql -uroot -p\\"\$MYSQL_ROOT_PASSWORD\\" -N --batch world"'
for f in sys.argv[1:]:
    stmts, cur = [], []
    for line in open(f, encoding='utf-8'):
        if line.startswith('--') or not line.strip():
            continue
        cur.append(line)
        if re.search(r";\s*(--.*)?$", line):
            stmts.append(''.join(cur)); cur = []
    sql = 'START TRANSACTION;\n' + ''.join(s + "SELECT 'RC', ROW_COUNT();\n" for s in stmts) + 'ROLLBACK;\n'
    r = subprocess.run(['ssh', '192.168.178.20', REMOTE], input=sql, capture_output=True, text=True, encoding='utf-8')
    rc = [int(l.split('\t')[1]) for l in r.stdout.splitlines() if l.startswith('RC')]
    print(f, 'statements', len(stmts), 'results', len(rc), 'rows', sum(rc), r.stderr.strip()[-200:] if r.returncode else '')
    for s, n in zip(stmts, rc):
        if n <= 0:
            print('   %d rows: %s' % (n, s.strip().splitlines()[0][:140]))
