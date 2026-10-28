import struct,sys
d=open(sys.argv[1],'rb').read()
magic=d[:4]; print(magic)
o=4+4+128
(rc,fc,rs,sts,th,lh,minid,maxid,loc)=struct.unpack_from('<9I',d,o); o+=36
flags,idx=struct.unpack_from('<HH',d,o); o+=4
(tfc,bpo,lcc,fsis,cds,pds,sc)=struct.unpack_from('<7I',d,o); o+=28
print(rc,fc,rs,sts,hex(flags),idx,tfc,sc)
secs=[]
for i in range(sc):
    s=struct.unpack_from('<Q8I',d,o); o+=40; secs.append(s)
fs=[struct.unpack_from('<hH',d,o+4*i) for i in range(fc)]; o+=4*fc
fsi=[struct.unpack_from('<HHIIIII',d,o+24*i) for i in range(fsis//24)]; o+=fsis
pal_off=o; o+=pds
com_off=o; o+=cds
# pallet per field
pal={};p=pal_off
com={};c=com_off
for i,f in enumerate(fsi):
    if f[3] in (3,4): pal[i]=p; p+=f[2]
    if f[3]==2:
        m={}
        for k in range(f[2]//8):
            rid,val=struct.unpack_from('<II',d,c+8*k); m[rid]=val
        com[i]=m; c+=f[2]
for s in secs:
    tk,fo,src,ssts,ore,idls,rel,omc,ctc=s
    rec=fo; strt=rec+src*rs; ids_off=strt+ssts
    ids=struct.unpack_from('<%dI'%src,d,ids_off) if idls else None
    for r in range(src):
        base=rec+r*rs
        bits=int.from_bytes(d[base:base+rs]+b'\0'*8,'little')
        vals=[]
        for i,f in enumerate(fsi):
            off,sz,ads,comp,a,b,cc=f
            if comp==0:
                v=(bits>>off)&((1<<sz)-1)
            elif comp in (1,5):
                v=(bits>>(off))&((1<<b)-1) if False else (bits>>a)&((1<<b)-1) if False else (bits>>off)&((1<<sz)-1)
                if comp==5 and v>>(sz-1): v-= (1<<sz)
            elif comp==2:
                v=None
            elif comp==3:
                ix=(bits>>off)&((1<<sz)-1); v=struct.unpack_from('<I',d,pal[i]+4*ix)[0]
            else:
                ix=(bits>>off)&((1<<sz)-1); v=list(struct.unpack_from("<%dI"%cc,d,pal[i]+4*cc*ix))
            vals.append(v)
        rid=ids[r] if ids else vals[idx]
        for i,f in enumerate(fsi):
            if f[3]==2: vals[i]=com[i].get(rid,f[4])
        print(rid,vals)
