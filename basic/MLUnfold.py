
# coding: utf-8

# In[1]:


from __future__ import print_function
from root_numpy import root2array, tree2array
import ROOT


# In[2]:


rfile = ROOT.TFile("TreeFile.root")
intree = rfile.Get('Tree')
intree.Print()
# and convert the TTree into an array
array = tree2array(intree,branches=["Gen","Reco","data"])
array.dtype.names = ('reco', "gen", "data")

print(array)


# In[3]:


import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import numpy as np
import pandas as pd
df = pd.DataFrame(array)

#print(df)
content_reco ,bin,patches = plt.hist(df['reco'], bins=15)
plt.ylabel('# events')
plt.xlabel("reco")
plt.show()

content_gen ,bin,patches = plt.hist(df['gen'], bins=15)
print(content_gen)
plt.ylabel('# events')
plt.xlabel("gen")
plt.show()

plt.hist2d(df['reco'],df['gen'], bins=10, range=[[0, 1000], [0, 1000]],norm=LogNorm())
plt.ylabel('# events')
plt.colorbar()
plt.show()



# In[4]:


batch_size = 1000
num_classes = 20
NBins = num_classes
epochs = 20


# In[5]:


import keras
gen=df["gen"].values
print(gen)
print(gen.shape)
g=np.trunc(gen/200)
print(g)
reco=df["reco"].values
print(reco)
print(reco.shape)
r=np.trunc(reco/200)
print(r)
data=df["data"].values
data=np.trunc(data/200)
data = data.reshape(data.shape[0],1,1)
gcat = keras.utils.to_categorical(g,NBins)
from sklearn.model_selection import train_test_split
r = r.reshape(r.shape[0],1,1)
r_train, r_test, g_train, g_test = train_test_split(r, gcat, test_size=0.22, random_state=42)


# In[6]:


from keras.models import Sequential
from keras.layers import Dense
from keras.models import Sequential
from keras.layers import Dense,Flatten
from keras import backend as K


# In[ ]:


def prepareModel(nvar=1, NBins=10, kappa=8):
    ''' Prepare KERAS-based sequential neural network with for ML unfolding. 
        Nvar defines number of inputvariables. NBins is number of truth bins. 
        kappa is an empirically tuned parameter for the intermediate layer'''
    model = Sequential()
    model.add(Dense(nvar,activation='linear',input_shape=(nvar,1)))
    model.add(Flatten())
    model.add(Dense(kappa*NBins**2,activation='relu'))
    
    # model.add(Dropout(0.25))
    # model.add(Dense(2*NBins,activation='relu'))
    # model.add(Dropout(0.5))
    model.add(Dense(NBins,activation='softmax'))
    model.compile(loss=keras.losses.categorical_crossentropy,
              optimizer=keras.optimizers.Adadelta(),
              metrics=['accuracy'])
    return model
model = prepareModel(1,NBins)


# In[ ]:


CallBack=keras.callbacks.TensorBoard(log_dir='./logs', histogram_freq=0, batch_size=32, write_graph=True, write_grads=False, write_images=False, embeddings_freq=0, embeddings_layer_names=None, embeddings_metadata=None)

h = model.fit(r_train,g_train,batch_size=batch_size,epochs=epochs,verbose=1,validation_data=(r_test,g_test), callbacks=[CallBack])


# In[ ]:


# save model,if needed
model.save("model.hdf5")


# In[ ]:


loss_and_metrics = model.evaluate(r_test, g_test, batch_size=128)
print(loss_and_metrics)


# In[ ]:


prob = model.predict(r_test, batch_size=128)
classes=prob.argmax(axis=-1)
np.set_printoptions(threshold=100000)
for i in range(20):    
    print(classes[i],g_test.argmax(axis=-1)[i])

