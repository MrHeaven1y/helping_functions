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
class Colimp:
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
        word=str(word)
        word = word.lower()
        alphabets={'a': 1,'b': 2,'c': 3,'d': 4,'e': 5,'f': 6,'g': 7,'h': 8,'i': 9,'j': 10,'k': 11,'l': 12,'m': 13,'n': 14,'o': 15,'p': 16,'q': 17,'r': 18,'s': 19,'t': 20,'u': 21,'v': 22,'w': 23,'x': 24,'y': 25,'z': 26,' ': 27,'_': 28,'%': 29,'-': 30,'@': 31,'/': 32,'[': 33,']': 34,'(': 35,')': 36,'=': 37,':': 38,';': 39,'?': 55,'<': 41,'>': 42,',': 43,'|': 44,'~': 45,'!': 46,'*': 47,'^': 48,'`': 49,"'": 50,'"': 51,'#': 52,'$': 53,'&': 54,'0':55,'1':56,'2':57,'3':58,'4':59,'5':60,'6':61,'7':62,'8':63,'9':64,'+':65,'.':66,'µ':67}
        set_alpha = set(alphabets)
        set_word  = set(word)
        inters = list(set_alpha.intersection(set_word))
        if inters==set() and set_word not in set_alpha:
            print('symbols are not matched')
            return None
        else:        
            def alpha_return(elem):
                return alphabets[elem]
            def word_return(word):
                words=[]
                for i in range(len(word)):
                    words.append(word[i:i+1])
                return words
            words = word_return(word)
            value_num = list(map(alpha_return,words))
            embeded=0
            for i in range(len(value_num)):
                p=10**(i+1)
                val = value_num[i]
                # if val>=10:
                    # embeded+=val/(p*10)
                # else:
                embeded+=val/p
            return embeded
    def embed(self,df,cols,replace_none=False, none_val='NaN'):
        value_none = self.embed_transform(none_val)
        names={}
        if type(cols) !='list':
            cols = list(cols)
        for col in cols:
            names[col] = list(map(self.embed_transform,df[col]))
        names_df= pd.DataFrame(names)
        if replace_none:
            names_df=names_df.replace(value_none,0.0)
            return names_df
        else:
            return names_df
    def color_conversion(self,w):
        l=len(w)
        p=self.embed_transform(w)
        k = p*(10**l)
        b=[]
        for i in range(3):
            b.append(k/(2+i))
        def color_codes(b):
            b1=255*np.sin(b)
            return int(b1)
        b_list=list(map(color_codes,b))
        b_vec=list(map(lambda x: -x if x<0 else x,b_list))
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


