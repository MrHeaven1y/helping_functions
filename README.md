# helping_functions
 some helping functions on data cleaning for machine learning

 ezz has many many helping functions which will help you to Analyse data and shortend your time on silly things like impute  missing data and differentiate categorical columns and numerical columns 

 soo in ezz 
 __init__:
Input Arguments:

impute_na (optional, default=False): Whether to perform imputation for missing values.
numerical_method (optional, default='mean'): Method to use for numerical imputation if impute_na is set to True.
Output Arguments:
None

get_dum:
Input Arguments:

df: DataFrame containing the data.
cate_col (optional): List of categorical column names.
num_col (optional): List of numerical column names.
target_col (optional): Target column name.
Output Arguments:
num_cols: List of numerical column names.
cate_cols: List of categorical column names.

make_dum:
Input Arguments:
df: DataFrame containing the data.
dum_cols (optional): List of categorical column names to make dummy variables for. If not provided, uses cate_cols from get_dum method.
dummy_na (optional, default=False): Whether to create a dummy variable for NA values.
dtype (optional, default=int): Data type for dummy variables.
drop_first (optional, default=True): Whether to drop the first level of each categorical variable.
concat (optional, default=True): Whether to concatenate the dummy variables with the original DataFrame.
drop_na_all (optional, default=True): Whether to drop columns where all values are NA.
Output Arguments:
DataFrame with dummy variables.

scale:
Input Arguments:
df: DataFrame containing the data.
num_cols: List of numerical column names to scale.
Output Arguments:
Scaled numerical columns.


impute_col:
Input Arguments:
df: DataFrame containing the data.
cate_cols (optional,default=None): List of categorical column names.
num_cols (optional,default=None): List of numerical column names.
method (optional, default='mean'): Method to use for imputation.
cat_imp (optional, default=False): Whether to perform imputation for categorical columns.
Output Arguments:
Imputed numerical columns.
Imputed categorical columns.


catcolimp:
Input Arguments:
df: DataFrame containing the data.
cate_cols: List of categorical column names.
Output Arguments:
Imputed categorical columns.
coreimp:
Input Arguments:

x: Independent variable (feature).
y: Dependent variable (target).
method (optional, default='lin_reg'): Method to use for imputation.
Output Arguments:
Dictionary containing the train score, test score, predicted values, and index of missing values.
