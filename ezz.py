import pandas as pd
import numpy as np
from sklearn.svm import SVR
from sklearn.linear_model import LinearRegression
from xgboost import XGBRegressor
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_squared_error
from sklearn.impute import SimpleImputer
from sklearn.preprocessing import MinMaxScaler
from functools import reduce
class colimp:
    def __init__(self, impute_na=False, numerical_method='mean'):
        self.impute_na = impute_na
        self.numerical_method = numerical_method
        global alphabets
    def get_dum(self, df, cate_col=None, num_col=None, target_col=''):
        if target_col:
            df = df.drop(target_col,axis=1)
        if cate_col is None and num_col is None:
            self.num_cols = list(df.select_dtypes(['int', 'float']).columns)
            self.cate_cols = list(df.select_dtypes(['object']).columns)
            print("numerical(1st arg) and categorical(2nd arg) columns are differentiate")
            return self.num_cols,self.cate_cols
        elif cate_col is None or num_col is None:
            if num_col is None and cate_col is not None:
                self.num_cols = list(df.select_dtypes(['int', 'float']).columns)
                self.cate_cols = cate_col
                print("numerical(1st arg) and categorical(2nd arg) columns are differentiate")
                return self.num_cols,self.cate_cols
            elif cate_col is None and num_col is not None:
                self.cate_cols = list(df.select_dtypes(['object']).columns)
                self.num_cols = num_col
                print("numerical(1st arg) and categorical(2nd arg) columns are differentiate")
                return self.num_cols,self.cate_cols
        elif cate_col is not None and num_col is not None:
            self.cate_cols = cate_col
            self.num_cols = num_col
            print("numerical(1st arg) and categorical(2nd arg) columns are differentiate")
            return self.num_cols,self.cate_cols
    def make_dum(self, df, dum_cols=None, dummy_na=False, dtype=int, drop_first=True, concat=False, drop_na_all=True):
        if dum_cols is None:
            dum_cols = self.cate_cols

        if drop_na_all:
            for col in df.columns:
                if df[col].isna().all():
                    df = df.drop(col, axis=1)
                    print('Succesfully removed {} column.'.format(col))

        dum = pd.get_dummies(df[dum_cols], drop_first=drop_first, dtype=dtype, dummy_na=dummy_na)
        if concat:
            dum_df = pd.concat([df, dum], axis=1)
            dum_df = dum_df.drop(dum_cols, axis=1)
            return dum_df
        else:
            return dum
    def scale(self,df,num_cols,method='minmax',inv_trans=False):
        if method=='div':
            mx =[]
            def find_max(num_col):
                m = max(df[num_col])
                mx.append(m)
                return mx
            def Scale(col,val):
                return df[col]/val
            def value(val):
                return df[val]
            max_list=list(map(find_max,num_cols))[0]
            return pd.DataFrame(map(Scale,num_cols,max_list)).T
        elif method=='minmax':
            global scaler
            scaler= MinMaxScaler().fit(df[num_cols])
            df[num_cols] = scaler.transform(df[num_cols])
            return df[num_cols]
        elif inv_trans:
            df[num_cols] = scaler.inverse_transform(df[num_cols])[:, [0]]
            return df[num_cols]
        else:
            print('Unknown method')
            return None
    def return_scaler(self):
        return scaler
    def extract_datetime(self,df,date_col='Date'):
        df[date_col] = pd.to_datetime(df[date_col])
        df['Year'] = df['Start_Date'].dt.year
        df['Month'] = df['Start_Date'].dt.month
        df['Day'] = df['Start_Date'].dt.day
    def extract_scale_cols(self,df,cols):
        ex_list=[]
        def Sum(a,b):
            return a+b
        def find_col(cols):
            l=list(df[cols])
            res = reduce(Sum,l[:10])
            return res>=100
        ex_list=list(filter(find_col,cols))
        return ex_list
    def impute_col(self,df,cate_cols=None, num_cols=None,method='mean',cat_imp=False):
        if cate_cols is None and num_cols is None:
            print('numerical columns and categorical columns is empty')
        elif cat_imp:
            df[cate_cols] = self.catcolimp(df,cate_cols)
            print('Done!')
            return df[cate_cols]
        elif num_cols is not None:
            if method == 'mean':
                imputer = SimpleImputer(strategy=method)
                df[num_cols] = imputer.fit_transform(df[num_cols])
                print('Done!')
                return df[num_cols]
            elif method== 'median':
                imputer = SimpleImputer(strategy=method)
                df[num_cols] = imputer.fit_transform(df[num_cols])
                print('Done!')
                return df[num_cols]
            elif method == 'constant':
                imputer = SimpleImputer(strategy=method)
                df[num_cols] = imputer.fit_transform(df[num_cols])
                print('Done!')
                return df[num_cols]
            else:
                print('Method is unknown')
                return None
    def catcolimp(self,df, cate_cols):
        imputer = SimpleImputer(strategy='most_frequent')
        df[cate_cols] = imputer.fit_transform(df[cate_cols])
        return df[cate_cols]
    def embed_transform(self,word):
            word = word.lower()
            alphabets={'a': 0.01,'b': 0.02,'c': 0.03,'d': 0.04,'e': 0.05,'f': 0.06,'g': 0.07,'h': 0.08,'i': 0.09,'j': 0.1,'k': 0.11,'l': 0.12,'m': 0.13,'n': 0.14,'o': 0.15,'p': 0.16,'q': 0.17,'r': 0.18,'s': 0.19,'t': 0.2,'u': 0.21,'v': 0.22,'w': 0.23,'x': 0.24,'y': 0.25,'z': 0.26,' ': 0.27,'_': 0.28,'%': 0.29,'-': 0.3,'@': 0.31,'/': 0.32,'[': 0.33,']': 0.34,'(': 0.35,')': 0.36,'=': 0.37,':': 0.38,';': 0.39,'?': 0.55,'<': 0.41,'>': 0.42,',': 0.43,'|': 0.44,'~': 0.45,'!': 0.46,'*': 0.47,'^': 0.48,'`': 0.49,'",': 0.5,'"': 0.51,'#': 0.52,'$': 0.53,'&': 0.54}
            set_alpha = set(alphabets)
            set_word  = set(word)
            inters = list(set_alpha.intersection(set_word))
            def d(elem):
                return alphabets[elem]
            value_num = list(map(d,inters))
            value_num.sort(reverse=True)
            count=0
            for i in range(1,len(value_num)+1):
                val = value_num[i-1]
                if val<=0.009:
                    count+=val
                elif val>=0.1:
                    count+=val/10
            return count
    def embed(self,df,cols,replace_none=False, none_val='NaN'):
        value_none = self.embed_transform(none_val)
        names={}
        if type(cols) !='list':
            cols = list(cols)
        for col in cols:
            names[col] = list(map(self.embed_transform,df[col]))
        names_df= pd.DataFrame(names)
        if replace_none:
            names_df.replace(value_none,0)
            return names_df
        else: return names_df
    def color_conversion(self,w):
        w=w.lower()
        l=len(w)
        p=self.embed_transform(w)
        k = p*(10**l)
        b=[]
        for i in range(3):
            b.append(int(k/(2+i)))
        def sigmoid(x):
            import numpy as np
            return 1/(1+np.exp(-x))
        def color_codes(b):
            b1=255*sigmoid(b)
            return int(b1)
        b_vec=list(map(color_codes,b))
        return b_vec
    def coreimp(self, x, y, method='lin_reg'):
        models=['lin_reg', 'svr', 'xgb']
        x = np.array(x)
        nv=np.isnan(x)
        count=0
        index=[]
        for i in nv:
            if i:
               index.append(count)
            count+=1
        x = (np.delete(x,index)).reshape(-1,1)
        y=np.delete(y,index)
        X_train, x_test, y_train, y_test = train_test_split(x, y, test_size=0.2, random_state=42)
        try:
            if method not in models:
                print('Unknown method')
                return None
            elif method == 'lin_reg':
                model = LinearRegression()
                model.fit(X_train, y_train)
                preds_train =  np.array(model.predict(X_train)).reshape(-1,1)
                preds_test = np.array(model.predict(x_test)).reshape(-1,1)
                scores = {'train score': mean_squared_error(preds_train,y_train,squared=False), 'test score': mean_squared_error(preds_test, y_test,squared=False), 'preds':model.predict(x[index])[0],'Index': index}
                return (scores)
            elif method == 'svr':
                model = SVR(C = 1, gamma = 'auto', epsilon = 1, kernel='linear')
                model.fit(X_train, y_train)
                preds_train =  np.array(model.predict(X_train)).reshape(-1,1)
                preds_test = np.array(model.predict(x_test)).reshape(-1,1)
                scores = {'train score': mean_squared_error(preds_train,y_train,squared=False), 'test score': mean_squared_error(preds_test, y_test,squared=False), 'preds':model.predict(x[index])[0],'Index': index}
                return (scores)
            elif method =='xgb':
                model = XGBRegressor()
                model.fit(X_train, y_train)
                preds_train =  np.array(model.predict(X_train)).reshape(-1,1)
                preds_test = np.array(model.predict(x_test)).reshape(-1,1)
                scores = {'train score': mean_squared_error(preds_train,y_train,squared=False), 'test score': mean_squared_error(preds_test, y_test,squared=False), 'preds':model.predict(x[index])[0],'Index': index}
                return (scores)
        except ValueError:
            print("There's no missing values in x")



