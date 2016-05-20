#####
# Projects
#####
all_projects=[
    'Base',
    'ISB',
    'SDM_Base',
    'SDM_Forget_1e-7',
    'SDM_Forget_5e-7',
    'SDM_Forget_1e-6',
    'SDM_Forget_5e-6',
    'SDM_Prune_7x_Context_0x',
    'SDM_Prune_7x',
    'SDM_Prune_6x',
    'SDM_Prune_5x',
    'SDM_Prune_4x',
    'SDM_Prune_3x',
    'SDM_Prune_2x',
    'SDM_Prune_1x',
    'SDM_Forget_5e-7_Prune_4x_Thin_.25',
    'SDM_Prune_7x_Context_2x',
    'SDM_Prune_7x_Context_4x',
    'SDM_Prune_7x_Context_8x',
    'SDM_Prune_7x_Context_16x',
    'SDM_Prune_7x_OrderContext',
    'SDM_Prune_7x_Context_2x_OrderContext',
    'SDM_Prune_7x_Context_4x_OrderContext',
    'SDM_Prune_7x_Context_8x_OrderContext',
    'SDM_Prune_7x_Context_16x_OrderContext',
    'SDM_Prune_7x_OrderContext_RemovePredictions',
    'SDM_Prune_7x_Context_2x_OrderContext_RemovePredictions',
    'SDM_Prune_7x_Context_4x_OrderContext_RemovePredictions',
    'SDM_Prune_7x_Context_8x_OrderContext_RemovePredictions',
    'SDM_Prune_7x_Context_16x_OrderContext_RemovePredictions',
    'SDM_Prune_4x_Batch',
    'SDM_Prune_4x_Degree_2_Batch_RemovePredictions',
    'TSDM_Prune_2x_Context_2x_OrderContext',
    'TSDM_Prune_2x_Context_4x_OrderContext',
    'TSDM_Prune_2x_Context_8x_OrderContext',
    'TSDM_Prune_2x_Context_16x_OrderContext',
    'TSDM_Prune_3x_Context_2x_OrderContext',
    'TSDM_Prune_3x_Context_4x_OrderContext',
    'TSDM_Prune_3x_Context_8x_OrderContext',
    'TSDM_Prune_3x_Context_16x_OrderContext',
    'TSDM_Prune_4x_Context_2x_OrderContext',
    'TSDM_Prune_4x_Context_4x_OrderContext',
    'TSDM_Prune_4x_Context_8x_OrderContext',
    'TSDM_Prune_4x_Context_16x_OrderContext',
    'TSDM_Prune_2x_Context_1x_OrderContext_NoCache',
    'TSDM_Prune_2x_Context_2x_OrderContext_NoCache',
    'TSDM_Prune_2x_Context_4x_OrderContext_NoCache',
    'TSDM_Prune_2x_Context_8x_OrderContext_NoCache',
    'TSDM_Prune_2x_Context_16x_OrderContext_NoCache',
    'TSDM_Prune_3x_Context_1x_OrderContext_NoCache',
    'TSDM_Prune_3x_Context_2x_OrderContext_NoCache',
    'TSDM_Prune_3x_Context_4x_OrderContext_NoCache',
    'TSDM_Prune_3x_Context_8x_OrderContext_NoCache',
    'TSDM_Prune_3x_Context_16x_OrderContext_NoCache',
    'TSDM_Prune_4x_Context_1x_OrderContext_NoCache',
    'TSDM_Prune_4x_Context_2x_OrderContext_NoCache',
    'TSDM_Prune_4x_Context_4x_OrderContext_NoCache',
    'TSDM_Prune_4x_Context_8x_OrderContext_NoCache',
    'TSDM_Prune_4x_Context_16x_OrderContext_NoCache'
]

def project_list_from_tag(tag):
    project_list = []
    for project in all_projects:
        if tag in project:
            project_list.append(project)
    return project_list

def pand(*args):
    project_set = set(all_projects)
    project_list = []
    for arg in args:
        project_set = project_set.intersection(arg_to_list(arg))
    for project in all_projects:
        if project in project_set:
            project_list.append(project)
    return project_list

def por(*args):
    project_set = set()
    project_list = []
    for arg in args:
        project_set = project_set.union(arg_to_list(arg))
    for project in all_projects:
        if project in project_set:
            project_list.append(project)
    return project_list

def pnot(arg):
    arg = arg_to_list(arg)
    project_list = []
    for project in all_projects:
        if project not in arg:
            project_list.append(project)
    return project_list


def arg_to_list(arg):
    if type(arg) in (str,unicode):
        return project_list_from_tag(arg)
    else:
        return arg


# Tags
sdm = 'SDM'
tsdm = 'TSDM'
isb = 'ISB'
base = 'Base'

# Other
no_cache = 'NoCache'
cache = pnot(no_cache)
separate_columns = 'SeparateColumns'

# Detraining
forget = '_Forget'
prune = '_Prune'
thin = '_Thin'
batch = '_Batch'
degree = '_Degree'

# Context
context = '_Context'
base_context = pnot(context)
no_context = pand(context,'0x')
extra_context = pand(context, pnot('0x'))

# Predicting
order_context = '_OrderContext'
order_confidence = pnot(order_context)
remove_predictions = '_RemovePredictions'
keep_predictions = pnot(remove_predictions)

