"""Read-only query helper: SQL goes through stdin to the mysql client in the DB container."""
import subprocess, sys

REMOTE = 'podman exec -i forever-db-1 sh -c "mysql -uroot -p\\"\\$MYSQL_ROOT_PASSWORD\\" -N --batch"'

def q(sql):
    r = subprocess.run(['ssh', '192.168.178.20', REMOTE], input=sql, capture_output=True,
                       text=True, encoding='utf-8')
    if r.returncode:
        raise RuntimeError(r.stderr)
    return [l.split('\t') for l in r.stdout.splitlines()]

if __name__ == '__main__':
    for row in q(sys.argv[1]):
        print('\t'.join(row))
