from pylab import plot, show, savefig, xlim, figure, \
                        hold, ylim, legend, boxplot, \
                        setp, axes, ioff, cla, clf, gca, \
                        table, close, normalize, cm
import matplotlib
from matplotlib.ticker import FuncFormatter
import pylab
import numpy as np
from pandas import *

# Project Specific Imports
from tools import *

ioff()

def addDataToAggregate(agg,data):
    keys = data.keys()
    for key in data_tags:
        agg[key]+=data[key] if key in keys else 0.0

#####
# Graph the distribution data for the following:
# Input:
#   * Project Name : String (e.g. BASE,ISB,SDM_Forget_0)
#   * Benchmarks   : List (i.e. use all, irregular or regular)
#   * Prefetchers  : List (i.e. SDMs or ISBs)
# Output:
#####
def statisticsBarGraph(simulator,projects,prefetchers,benchmarks,statistics,
                        x_tags=[bench_t],z_tags=[pref_t],
                        projects_title="Selected",projects_name=""):
    print projects
    add_projects, add_prefetchers = getConstScrapingInfo(statistics)
    weights = simulator.getSimpointWeights(benchmarks)
    project_settings = getProjectSettings()
    # Scrape data of the given parameters.
    data = simulator.scrape_data(projects+add_projects,
                                    prefetchers+add_prefetchers,
                                    benchmarks)

    statistics = [stat for stat in statistics if stat.supports_sim(simulator)]

    # Choose which options go on the X and Z axes.
    # The Y axis will always be the statistic.
    #axes_options = {
    #    proj_t : projects,
    #    pref_t : prefetchers,
    #    bench_t : benchmarks
    #}
    #original_x_list = axes_options[x_tag]
    #original_z_list = axes_options[z_tag]

    data = statsFromData(data,statistics,projects,prefetchers,benchmarks,weights)

    x_set = set()
    z_set = set()
    organized_stats = {}
    for elem in data:
        elem.update(project_settings[elem[proj_t]])
        x = tuple([elem[x_tag] for x_tag in x_tags])
        z = tuple([elem[z_tag] for z_tag in z_tags])
        x_set.add(x)
        z_set.add(z)
        if not z in organized_stats.keys():
            organized_stats[z] = {x:{}}
        elif not x in organized_stats[z]:
            organized_stats[z][x] = {}
        for stat in statistics:
            stat_tag = stat.name
            if not stat_tag in organized_stats[z][x]:
                organized_stats[z][x][stat_tag] = []
            try:
                organized_stats[z][x][stat_tag].append(elem[stat_tag])
            except:
                pass

    #  Create list of used x and z axis items
    x_list = sorted(x_set,create_compare(x_tags))
    x_str_list = ['\n'.join([str(x_i) for x_i in x]) for x in x_list]
    z_list = sorted(z_set,create_compare(z_tags))
    z_str_list = ['\n'.join([str(z_i) for z_i in z]) for z in z_list]
    #z_list = [z_item for z_item in original_z_list if z_item in z_set]

    width = 1       # the width of the bars
    z_len = len(z_list)
    x_len = len(x_list)
    x_pad = 2 if x_len >=12 else 1
    inds = [i*width*(z_len+1)+x_pad for i in range(0,x_len)]
    x_end = inds[-1] + width*(z_len)+x_pad

    # Find the shared non-standard project settings
    shared_settings = {}
    for s in project_settings_tags:
        shared_settings[s] = set()
    for proj in projects:
        p = project_settings[proj]
        for (s,v) in p.iteritems():
            shared_settings[s] |= set([v])
    base_settings_dict = {}
    for tag,values in shared_settings.iteritems():
        if len(values) == 1:
            value = list(values)[0]
            if not value == default_project_settings[tag]:
                base_settings_dict[tag] = value

    base_settings_arr = ['Shared Settings\n']
    for (k,v) in base_settings_dict.iteritems():
        base_settings_arr.append(k+': '+str(v))
    base_settings = '\n'.join(base_settings_arr)


    # Aggregate Data by axis
    # Keep track of used axis items, so unused items
    # don't have to be graphed.
    #for elem in data:
    #    print elem[z_tag], elem[x_tag]
    #    z_set.add(elem[z_tag])
    #    x_set.add(elem[x_tag])
    #    addDataToAggregate(aggregated_data[elem[z_tag]][elem[x_tag]],elem)

    # Make boxplots
    #hold(True)

    # For Table Aggregations
    table_stat_agg = {}

    fig = figure()
    ax = axes()

    # Setting fonts and sizes so that everything fits and looks good
    font_size = 'x-small'
    #if len(x_list) <= 12:
    #    font_size = 'x-small'

    box = ax.get_position()

    # Making gap big enough for legend to fit
    legend_strs = ['\n'.join(z_tags)] + z_str_list
    leg_len = 0
    for z_item in legend_strs:
        for z_item_s in z_item.split('\n'):
            leg_len = max(leg_len,len(z_item_s))
    leg_pad = max(0,(leg_len-6)*.008)

    xtick_len = 0
    for x_item in x_str_list:
        for x_item_s in x_item.split('\n'):
            xtick_len = max(xtick_len,len(x_item_s))
    xtick_pad = max(0,xtick_len*.006)
    new_box = [box.x0-.02,box.y0+xtick_pad,box.width-leg_pad,box.height-xtick_pad]

    close()

    for statistic in statistics:
        fig = figure()
        ax = axes()
        stat_tag = statistic.name

        # Create Aggregation Dictionary


        #for z_item in z_set:
        #    z_dict = {}
        #    for x_item in x_set:
        #        z_dict[x_item] = []
        #    organized_stats[z_item] = z_dict

        # Check that the simulator generates the data necessary
        # for the chosen Statistic.


        legend_ar = [ax.bar([0],[0],color='white')[0]]

        table_z_agg = {}

        # Creating and graphing Bars
        i = 0
        for z_item in z_list:
            color = colorArr[i]
            #print stats[z_item]
            data_list = []
            max_len = 0
            for x_item in x_list:
                if x_item not in organized_stats[z_item]:
                    data_list.append(0)
                    continue
                if stat_tag not in organized_stats[z_item][x_item]:
                    data_list.append(0)
                    continue

                ind_stats = organized_stats[z_item][x_item][stat_tag]
                stats_len = len(ind_stats)
                if stats_len>max_len:
                    data_list = [0]*len(data_list)
                    max_len = stats_len
                if stats_len==max_len:
                    data_list.append(statistic.aggregate(ind_stats))
                else:
                    data_list.append(0)
            indices = [ind + i for ind in inds]
            bars = ax.bar(indices,data_list,width,color=color) #Map[z_tag][z_item])
            i+=1

            lx = axes()
            legend_ar.append(bars[0])

        # Data for the aggregation table
        #for z_item in z_list:
        #    x_agg = []
        #    for x_item in organized_stats[z_item]:
        #        x_agg += x_item[stat_tag]
            # table_z_agg[z_item] = '%.2f%%' % (statistic.aggregate(x_agg)*100)
            #table_z_agg[z_item] = statistic.aggregate(x_agg)

        # Adding to total table aggregation data
        #table_stat_agg[stat_tag] = table_z_agg



        # Setting labels and titles, and ticks
        ax.set_position(new_box)
        title = statistic.title(simulator.name().title(), \
                                projects_title, \
                                name_benchmarks(benchmarks).title())
        box_height = new_box[1]+new_box[3]
        ax.set_title(title,loc='center',y=1.03)
        ax.text(ax.get_xlim()[1]+1.25,ax.get_ylim()[1]*.9,base_settings,
                ha='left', va='top', size='x-small',
                bbox=dict(edgecolor='black',
                          facecolor='white'))
        ax.set_ylabel(statistic.y_label())
        ax.set_xlabel('\n'.join(x_tags))
        ax.set_xticks([ind + (z_len)/2.0 for ind in inds])
        ax.set_xlim(0,x_end)
        ax.set_xticklabels(x_str_list,rotation=60,ha='center',size=font_size)
        pylab.rcParams.update({'legend.fontsize':8, 'legend.linewidth':10})

        # Legend
        ax.legend(legend_ar,legend_strs,bbox_to_anchor=(1, .5),loc='center left')

        gca().yaxis.set_major_formatter(FuncFormatter(to_percent))


        if projects_name=="":
            show()
        else:
            filename = statistic.filename(simulator.name(), \
                                            projects_name, \
                                            name_benchmarks(benchmarks));
            savefig('../graphs/'+filename+'.png',dpi=250)

        close()


    # Make the overall statistics table
    table_agg = [[table_stat_agg[stat.name][z_item] for stat in statistics] for z_item in z_list]
    generate_table(table_agg,z_list,[stat.name for stat in statistics])

def generate_table(table_data,rows,columns):
    df = DataFrame(table_data, index=rows, columns=columns)
    base = 0
    colorings = []
    #colorings.append([base]*(len(rows)+1))
    for column in columns:
        vals = df[column].values
        normal = normalize(vals.min()-1, vals.max()+1)
        m = cm.ScalarMappable(norm=matplotlib.colors.Normalize(vmin=vals.min(), vmax=vals.max()))
        m.set_cmap("YlGn")
        col_colors = m.to_rgba(vals)
        print col_colors
        colorings.append(col_colors)

        #colorings.append(list(np.transpose(cm.hot(normal(vals)))[1]))
    colorings = np.transpose(colorings,(1,0,2))
    print colorings.shape

    fig = figure(figsize=(15,8))
    ax = fig.add_subplot(111, frameon=True, xticks=[], yticks=[])
    #print colorings
    vals = df.values

    the_table=table(cellText=vals, rowLabels=df.index, colLabels=df.columns,
                        colWidths = [0.13]*vals.shape[1],
                        loc='center',
                        cellColours=colorings,
                        fontsize=15)

    #nrows, ncols = len(rows)+1, len(columns)+1
    #hcell, wcell = 1, 1
    #hpad, wpad = .5, 0
    #fig = figure(figsize=(ncols*wcell+wpad, nrows*hcell+hpad))
    #fig = figure()
    #ax = fig.add_subplot(111)
    #ax.axis('off')
    #ax.table(cellText=table_data,
    #      rowLabels=rows,
    #      colLabels=columns,
    #      loc='center')
    show()
    close()

def to_percent(y, position):
    # Ignore the passed in position. This has the effect of scaling the default
    # tick locations.
    s = '%.0f' % (100 * y)

    # The percent symbol needs escaping in latex
    if matplotlib.rcParams['text.usetex'] is True:
        return s + r'$\%$'
    else:
        return s + '%'
