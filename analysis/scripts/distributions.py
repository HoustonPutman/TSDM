from pylab import plot, show, savefig, xlim, figure, \
                        hold, ylim, legend, boxplot, \
                        setp, axes, ioff, cla, clf
import pylab
import numpy as np

# Project Specific Imports
from tools import *

ioff()



# function for setting the colors of the box plot groups
def setBoxColors(bp,categories,lw):
    color_map = {}
    for i in range(0,len(categories)):
        print i
        c = colorArr[i]
        print bp['boxes']
        setp(bp['boxes'][i], color=c, linewidth=lw)
        setp(bp['caps'][2*i], color=c, linewidth=lw)
        setp(bp['caps'][2*i + 1], color=c, linewidth=lw)
        setp(bp['whiskers'][2*i], color=c, linewidth=lw)
        setp(bp['whiskers'][2*i + 1], color=c, linewidth=lw)
        setp(bp['fliers'][2*i], color=c, linewidth=lw)
        setp(bp['fliers'][2*i + 1], color=c, linewidth=lw)
        setp(bp['medians'][i], color=c, linewidth=lw)

#####
# Graph the distribution data for the following:
# Input:
#   * Project Name : String (e.g. BASE,ISB,SDM_Forget_0)
#   * Benchmarks   : List (i.e. use all, irregular or regular)
#   * Prefetchers  : List (i.e. SDMs or ISBs)
# Output:
#####
def contentDistBoxPlots(projects,prefetchers,benchmarks,title,name,show_plot=False):

    # Consolidate and select correct data
    #dists = getDistributions(proj,prefetchers,benchmarks)
    dists = getSniperDistributions(projects,prefetchers,benchmarks)

    z_tag = proj_t
    x_tag = pref_t

    axes_options = {
        proj_t : projects,
        pref_t : prefetchers,
        bench_t : benchmarks
    }
    original_x_list = axes_options[x_tag]
    original_z_list = axes_options[z_tag]

    organized_stats = {}
    for z_item in original_z_list:
        z_dict = {}
        for x_item in original_x_list:
            z_dict[x_item] = []
        organized_stats[z_item] = z_dict

    max_y = 0
    x_set = set()
    z_set = set()
    for elem in dists:
        if dist_t in elem:
            dist = elem[dist_t]
            x_set.add(elem[x_tag])
            z_set.add(elem[z_tag])
            agg_list = organized_stats[elem[z_tag]][elem[x_tag]]
            for d,count in dist.iteritems():
                if d == 0:
                    continue
                for i in range(0,count):
                    agg_list.append(d)
                max_y = max(max_y,d)

    x_list = [x_item for x_item in original_x_list if x_item in x_set]
    z_list = [z_item for z_item in original_z_list if z_item in z_set]

    #bench_dict = {}
    #max_y = 0
    #for bench in benchmarks:
    #    bench_dict[bench]={}
    #for pref,bdists in dists.iteritems():
    #    if not pref in prefetchers:
    #        continue
    #    for bench,dist in bdists.iteritems():
    #        if not bench in benchmarks:
    #            continue
    #        complete_dist = [] #np.array([])
    #        for d,count in dist.iteritems():
    #            if d==0:
    #                continue
    #            for i in range(0,count):
    #                complete_dist.append(d)#np.append(complete_dist,d)
    #            max_y = max(max_y,d)
    #        bench_dict[bench][pref]=complete_dist

    # Make boxplots
    #fig = figure()
    ax = axes()
    hold(True)

    font_size = 'xx-small'
    line_width = 1
    if len(benchmarks) <= 12:
        font_size = 'small'
        line_width = 2

    position = 1
    num_bench_graphed = 0
    for z_item in z_list:
        pdict = organized_stats[z_item]
        if len(pdict)<1:
            continue
        num_bench_graphed+=1
        pos_ar = range(position,position+len(x_list))
        #print pos_ar
        position += len(x_list) + 1
        data = [pdict[x_item] for x_item in x_list]
        bp = ax.boxplot(data,positions = pos_ar, widths = 0.6, sym='')
        setBoxColors(bp,x_list,line_width)

    group_x_len = len(x_list) + 1
    xend = (group_x_len)*num_bench_graphed
    print xend
    print ''
    xlim(0,xend)
    #ylim(0,2000)

    ax.set_xticks([-1]+[(i-.5)*group_x_len for i in range(1,num_bench_graphed+1)]+[group_x_len*num_bench_graphed+1])
    ax.set_xticklabels(['']+x_list+[' '], rotation=60, ha='center', size=font_size)
    ax.set_xlabel("Benchmarks")
    ax.set_ylabel("Content Size Distribution")
    ax.set_yscale('log')
    ax.set_title(title,y=1.02)
    box = ax.get_position()
    ax.set_position([box.x0-.02,box.y0+.1,box.width,box.height*.9])

    pylab.rcParams.update({'legend.fontsize':8, 'legend.linewidth':10})
    # Legend
    leg_lines = []
    i = 0
    for z_item in z_list:
        ax.plot([],c=colorArr[i],label=z_item, linewidth=4)
        i+=1
    legend(bbox_to_anchor=(1, .5),loc='center left')
    if show_plot:
        show()
    else:
        savefig('../graphs/'+name+'.png',dpi=200)

    clf()
