import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
OUT="/tmp/claude-1000/-home-abdurrahman-Documents-projects-compute-engine/9f28a0f1-109a-463b-8a91-0ec3c6c2fd96/scratchpad/fig/"
INK="#0b0b0b"; SEC="#52514e"; BLUE="#2a78d6"; ORANGE="#eb6834"; AQUA="#1baf7a"; GRAY="#e9e8e4"; LBLUE="#dbe8f8"; LOR="#fbe3d8"; LAQ="#d6f1e6"
plt.rcParams.update({"font.family":"DejaVu Sans","font.size":9})
def box(ax,x,y,w,h,t,fc=GRAY,ec=SEC,fs=9,bold=False):
    ax.add_patch(FancyBboxPatch((x,y),w,h,boxstyle="round,pad=0.02,rounding_size=0.08",fc=fc,ec=ec,lw=1))
    ax.text(x+w/2,y+h/2,t,ha="center",va="center",fontsize=fs,color=INK,weight="bold" if bold else "normal",wrap=True)
def arr(ax,x1,y1,x2,y2,c=SEC,ls="-",rad=0):
    ax.add_patch(FancyArrowPatch((x1,y1),(x2,y2),arrowstyle="-|>",mutation_scale=10,color=c,lw=1.2,ls=ls,connectionstyle=f"arc3,rad={rad}"))

# Fig 1: overall software architecture / flow
fig,ax=plt.subplots(figsize=(9,4.2)); ax.set_xlim(0,18); ax.set_ylim(0,8.4); ax.axis("off")
ax.text(1.6,8.0,"Program preparation",ha="center",color=SEC,fontsize=9,weight="bold")
ax.text(9,8.0,"Simulation core",ha="center",color=SEC,fontsize=9,weight="bold")
ax.text(16.2,8.0,"Outputs",ha="center",color=SEC,fontsize=9,weight="bold")
box(ax,0.2,6.0,2.8,1.3,"RV32I assembly\n(.s)")
box(ax,0.2,3.9,2.8,1.3,"Two-pass\nassembler",LBLUE,BLUE)
box(ax,0.2,1.8,2.8,1.3,"C program +\ncrt0.S + linker.ld")
box(ax,0.0,0.1,3.2,1.1,"RISC-V GCC\n(-march=rv32i)",fs=8)
arr(ax,1.6,6.0,1.6,5.2); arr(ax,1.6,1.8,1.6,1.2)
box(ax,4.0,3.9,2.6,1.3,"Custom loader\n(-a)",LBLUE,BLUE); box(ax,4.0,0.9,2.6,1.3,"ELF loader\n(-e)",LBLUE,BLUE)
arr(ax,3.0,4.55,4.0,4.55); arr(ax,3.0,0.65,4.0,1.4)
box(ax,4.0,6.2,2.6,1.2,"JSON config\n(-c, cJSON)",LOR,ORANGE)
box(ax,7.4,0.4,6.8,7.0,"",fc="#f6f6f4",ec=SEC)
box(ax,7.8,5.6,6.0,1.4,"5-stage pipeline  IF | ID | EX | MA | WB\nforwarding · hazard detection\nbranch predictor",LBLUE,BLUE,fs=8)
box(ax,7.8,3.3,2.8,1.6,"L1-I / L1-D\nL2 → L3\n(timing model)",LAQ,AQUA,fs=8.5)
box(ax,11.0,3.3,2.8,1.6,"DRAM\nuint8_t array\nlittle-endian",LAQ,AQUA,fs=8.5)
box(ax,7.8,0.8,6.0,1.7,"Statistics counters\n(hits/misses, stalls, flushes, forwards, CPI)",LOR,ORANGE,fs=8.5)
arr(ax,6.6,4.3,7.4,4.3); arr(ax,6.6,1.55,7.4,1.9); ax.text(7.0,2.9,"load\nimage",fontsize=7,color=SEC,ha="center")
arr(ax,6.6,6.8,7.8,6.4)
arr(ax,9.2,5.6,9.2,4.9); arr(ax,10.6,4.1,11.0,4.1); arr(ax,10.8,2.5,10.8,3.3,ls="--")
box(ax,15.0,6.2,2.8,1.1,"Pipeline visual\npipevis.txt",fs=8); box(ax,15.0,4.7,2.8,1.1,"Register dumps\n(-r)",fs=8)
box(ax,15.0,3.2,2.8,1.1,"Memory dump\n(-m)",fs=8); box(ax,15.0,1.7,2.8,1.1,"I/D trace files\n(-t) → cachesim",fs=8)
box(ax,15.0,0.2,2.8,1.1,"Statistics report\n(stdout)",fs=8)
for y in (6.75,5.25,3.75,2.25,0.75): arr(ax,14.2,min(max(y,1.0),6.8),15.0,y)
plt.savefig(OUT+"fig_arch.png",dpi=200,bbox_inches="tight"); plt.close()

# Fig 2: pipeline datapath abstraction
fig,ax=plt.subplots(figsize=(9,3.6)); ax.set_xlim(0,20); ax.set_ylim(0,9); ax.axis("off")
st=[("IF","PC, +4 adder\nL1-I fetch",0.3),("ID","decode / control\nreg read, imm gen\nBTFN/T/NT predict",4.3),("EX","ALU, branch\ncompare, target\nforward_ex()",8.3),("MA","L1-D access\nbranch resolve\nforward_ma()",12.3),("WB","register\nwrite-back",16.3)]
for n,t,x in st:
    box(ax,x,3.0,2.9,2.8,f"{n}\n\n{t}",LBLUE,BLUE,fs=8)
regs=["IF/ID","ID/EX","EX/MA","MA/WB"]
for i,r in enumerate(regs):
    x=3.35+4*i; ax.add_patch(FancyBboxPatch((x,2.6),0.55,3.6,boxstyle="square,pad=0",fc=LOR,ec=ORANGE)); ax.text(x+0.27,6.35,r,ha="center",fontsize=8,color=INK)
    arr(ax,x-0.15,4.4,x,4.4); arr(ax,x+0.55,4.4,x+0.95,4.4)
ax.add_patch(FancyBboxPatch((19.3,2.6),0.55,3.6,boxstyle="square,pad=0",fc=GRAY,ec=SEC)); ax.text(19.57,6.35,"WB/IF",ha="center",fontsize=8)
arr(ax,19.2,4.4,19.3,4.4)
arr(ax,13.7,3.0,9.7,3.0,c=ORANGE,rad=0.35); ax.text(11.7,1.75,"EX/MA, MA/WB, WB/IF → EX forwarding",ha="center",fontsize=7.5,color=SEC)
arr(ax,13.7,5.8,1.75,5.8,c=AQUA,rad=0.25); ax.text(9.5,8.3,"mispredict / jump: EX-flush, PC ← target (resolved in MA)",ha="center",fontsize=7.5,color=SEC)
arr(ax,5.75,5.8,1.75,5.8,c=BLUE,ls="--",rad=0.4); ax.text(2.3,7.6,"predict-taken: ID-flush",ha="center",fontsize=7.5,color=SEC)
box(ax,0.3,0.2,2.9,1.2,"L1-I cache",LAQ,AQUA,fs=8); box(ax,12.3,0.2,2.9,1.2,"L1-D cache",LAQ,AQUA,fs=8)
box(ax,4.3,0.2,7.0,1.2,"L2 → L3 → DRAM (shared, timing only)",LAQ,AQUA,fs=8)
arr(ax,1.75,3.0,1.75,1.4); arr(ax,13.75,3.0,13.75,1.4)
plt.savefig(OUT+"fig_pipeline.png",dpi=200,bbox_inches="tight"); plt.close()
print("ok")
