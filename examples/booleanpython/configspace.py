import numpy as np
import itertools

def config ( parameter ):
    amax=12.0
    bmax=2.5
    cmax=2.5
    dmax=2.5
    A=parameter[0]
    B=parameter[1]
    C=parameter[2]
    D=parameter[3]
    xmlstr = genStringForFixedParams ( test4DExample1,(A,B,C,D,amax,bmax,cmax,dmax) )
    return xmlstr



def test4DExample1(A,B,C,D,Amax,Bmax,Cmax,Dmax):
    '''
    Example inputs.

    '''
    # define the interactions between variables
    variables = ['x','y1','y2','z']
    affectedby = [['x','y2','z'],['x'],['y1'],['x']]
    # give the thresholds for each interaction
    thresholds = [[2,1,1],[3],[1],[1]]
    # give the maps and amplitudes of each interaction (upper and lower bounds for parameter search)
    maps = [[(0,0,0),(1,0,0),(0,1,0),(1,1,0),(0,0,1),(1,0,1),(0,1,1),(1,1,1)],[(0,),(1,)],[(0,),(1,)],[(0,),(1,)]]
    ampfunc = lambda A,B,C,D: [[0,4.25,A,A+4.25,0,1.0625,A*0.25,(A+4.25)*0.25],[0,C],[0,D],[0,B]]
    amps = ampfunc(A,B,C,D)
    loweramplitudes = amps
    upperamplitudes = amps
    # give the natural decay rates of the species (upper and lower bounds for parameter search)
    lowerdecayrates = [-1,-1,-1,-1]
    upperdecayrates = [-1,-1,-1,-1] #[-0.5,-0.5,-0.5]
    # give the endogenous production rates. 
    productionrates = [0.1,0.5,0.5,0.5] 
    # get upper and lower bounds
    bigamps = ampfunc(Amax,Bmax,Cmax,Dmax)
    upperbounds = 1.1*((np.array([np.max(u) for u in bigamps]) + np.array(productionrates)) / np.abs(np.array(upperdecayrates))) # calculate upper bounds of domains
    lowerbounds = np.zeros(upperbounds.shape)
    return variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds

def test4DExample2(A,B,Amax,Bmax):
    '''
    Example inputs.

    '''
    # define the interactions between variables
    variables = ['x','y1','y2','z']
    affectedby = [['x','y2','z'],['x'],['y1'],['x']]
    # give the thresholds for each interaction
    thresholds = [[2,1,1],[1],[1],[3]]
    # give the maps and amplitudes of each interaction (upper and lower bounds for parameter search)
    maps = [[(0,0,0),(1,0,0),(0,1,0),(1,1,0),(0,0,1),(1,0,1),(0,1,1),(1,1,1)],[(0,),(1,)],[(0,),(1,)],[(0,),(1,)]]
    ampfunc = lambda A,B: [[0,4.25,A,A+4.25,0,1.0625,A*0.25,(A+4.25)*0.25],[0,2.5],[0,2.5],[0,B]]
    amps = ampfunc(A,B)
    loweramplitudes = amps
    upperamplitudes = amps
    # give the natural decay rates of the species (upper and lower bounds for parameter search)
    lowerdecayrates = [-1,-1,-1,-1]
    upperdecayrates = [-1,-1,-1,-1] #[-0.5,-0.5,-0.5]
    # give the endogenous production rates. 
    productionrates = [0.1,0.5,0.5,0.5] 
    # get upper and lower bounds
    bigamps = ampfunc(Amax,Bmax)
    upperbounds = 1.1*((np.array([np.max(u) for u in bigamps]) + np.array(productionrates)) / np.abs(np.array(upperdecayrates))) # calculate upper bounds of domains
    lowerbounds = np.zeros(upperbounds.shape)
    return variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds

def test4DExample3(A,B,Amax,Bmax):
    '''
    Example inputs.

    '''
    # define the interactions between variables
    variables = ['x','y1','y2','z']
    affectedby = [['x','y2','z'],['x'],['y1'],['x','y2']]
    # give the thresholds for each interaction
    thresholds = [[2,1,1],[3],[1],[1,2]]
    # give the maps and amplitudes of each interaction (upper and lower bounds for parameter search)
    maps = [[(0,0,0),(1,0,0),(0,1,0),(1,1,0),(0,0,1),(1,0,1),(0,1,1),(1,1,1)],[(0,),(1,)],[(0,),(1,)],[(0,0),(1,0),(0,1),(1,1)]]
    ampfunc = lambda A,B: [[0,4.25,A,A+4.25,0,1.0625,A*0.25,(A+4.25)*0.25],[0,2.5],[0,2.5],[0,B,0.4,B+0.4]]
    amps = ampfunc(A,B)
    loweramplitudes = amps
    upperamplitudes = amps
    # give the natural decay rates of the species (upper and lower bounds for parameter search)
    lowerdecayrates = [-1,-1,-1,-1]
    upperdecayrates = [-1,-1,-1,-1] #[-0.5,-0.5,-0.5]
    # give the endogenous production rates. 
    productionrates = [0.1,0.5,0.5,0.5] 
    # get upper and lower bounds
    bigamps = ampfunc(Amax,Bmax)
    upperbounds = 1.1*((np.array([np.max(u) for u in bigamps]) + np.array(productionrates)) / np.abs(np.array(upperdecayrates))) # calculate upper bounds of domains
    lowerbounds = np.zeros(upperbounds.shape)
    return variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds

def xyz3DExample():
    '''
    Example inputs.

    '''
    # define the interactions between variables
    variables = ['x','y','z']
    affectedby = [['x','y','z'],['x'],['x','y']]
    # give the thresholds for each interaction
    thresholds = [[0.25,0.5,0.5],[0.5],[0.75,0.5]]
    # give the maps and amplitudes of each interaction (upper and lower bounds for parameter search)
    maps = [[(0,0,0),(1,0,0),(0,1,0),(0,0,1),(1,1,0),(1,0,1),(0,1,1),(1,1,1)],[(0,),(1,)],[(0,0),(0,1),(1,0),(1,1)]]
    loweramplitudes = [[0.5,0.75,1.0,0.0,1.25,0.25,0.5,0.75],[0.0,0.5],[0.0,0.5,0.5,1.0]]
    upperamplitudes = [[1.5,2.25,3.0,0.0,3.75,0.75,1.5,2.25],[0.0,1.5],[0.0,1.5,1.5,3.0]]
    # give the natural decay rates of the species (upper and lower bounds for parameter search)
    lowerdecayrates = [-1.5,-0.75,-0.75]
    upperdecayrates = [-0.5,-0.25,-0.25]
    # give the endogenous production rates. 
    productionrates = [0.1,0.1,0.1] 
    # get upper and lower bounds
    upperbounds = 1.1*((np.array([np.max(u) for u in upperamplitudes]) + np.array(productionrates)) / np.abs(np.array(upperdecayrates))) # calculate upper bounds of domains
    lowerbounds = np.zeros(upperbounds.shape)
    return variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds

def genStringForFixedParams(model,paramtuple):
    variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds = model(*paramtuple)
    xmlstr = convertInputsToXML(variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds)
    return xmlstr

def convertInputsToXML(variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds):
    thresh,ainds,pr = makeParameterArrays(variables, affectedby, thresholds, productionrates)
    doms = getDomains(thresh,upperbounds)
    lsigs,usigs = getSigmas(doms,thresh,loweramplitudes,upperamplitudes,pr,ainds,maps)
    xmlstr = generateXML(lowerbounds,upperbounds,lowerdecayrates,upperdecayrates,doms,lsigs,usigs)
    return xmlstr

def makeParameterArrays(variables, affectedby, thresholds, productionrates):
    '''
    Put model parameters into square arrays, where the column 
    index indicates the source variable and the row index 
    indicates the target variable.

    The outputs thresh, lamp, uamp, and rep are len(variables) x len(variables)
    arrays with threshold values, amplitudes, and repressor 
    identities filled in between the appropriate source-target 
    interactions. All other array values are zero. The output
    pr is simply the list productionrates converted to an array.

    '''
    N = len(variables)
    thresh = np.zeros((N,N))
    ainds = []
    for j,a in enumerate(affectedby): #j is index of target
        at = []
        for k,t in enumerate(a):
            i =  variables.index(t) #i is index of source
            at.append(i)
            thresh[j,i] = thresholds[j][k]
        ainds.append(at)
    pr = np.array(productionrates)
    return thresh, ainds, pr

def getDomains(thresh,upperbounds):
    '''
    Find the domain bounds for each 
    regular domain (area between thresholds).

    '''
    dp = []
    for j in range(thresh.shape[1]):
        tl = list(np.unique(thresh[:,j])) + [upperbounds[j]]
        if tl[0] != 0:
            tl = [0] + tl
        dp.append( [tl[k:k+2] for k in range(len(tl[:-1]))] )
    # return the bounds for each regular domain 
    return np.array(list(itertools.product(*dp)))

def getSigmas(doms,thresh,lamp,uamp,pr,ainds,maps):
    '''
    Find the sigma bounds in each regular domain.

    '''
    lsigs = []
    usigs = []
    for i in range(doms.shape[0]):
        ls=[]
        us=[]
        for j,a in enumerate(ainds):
            bmap = tuple( (np.mean(doms[i,:,:],1) > thresh[j,:]).astype(int)[a] ) # get the binary map for target j
            for k,m in enumerate(maps[j]):
                if m == bmap:
                    ls.append(lamp[j][k]+pr[j])
                    us.append(uamp[j][k]+pr[j])
                    break
        lsigs.append(ls)
        usigs.append(us)
    return lsigs,usigs

def generateXML(lowerbounds,upperbounds,lowerdecayrates,upperdecayrates,doms,lsigs,usigs):
    xmlstr = "<atlas>\n"
    xmlstr += "  <dimension> " + str(len(lowerbounds)) + " </dimension>\n"
    xmlstr += "  <phasespace>\n"
    xmlstr += "    <bounds>\n"
    xmlstr += "      <lower> " + "  ".join([str(l) for l in lowerbounds]) + " </lower>\n"  
    xmlstr += "      <upper> " + "  ".join([str(u) for u in upperbounds]) + " </upper>\n"  
    xmlstr += "    </bounds>\n"
    xmlstr += "  </phasespace>\n"
    xmlstr += "  <gamma>\n"
    xmlstr += "    <lower> " + "  ".join([str(l) for l in lowerdecayrates]) + " </lower>\n"
    xmlstr += "    <upper> " + "  ".join([str(u) for u in upperdecayrates]) + " </upper>\n"
    xmlstr += "  </gamma>\n"
    xmlstr += "  <listboxes>\n"
    for k in range(doms.shape[0]):
        xmlstr += "    <box> <!-- box : " + str(k) + " -->\n"
        xmlstr += "      <bounds>\n"
        xmlstr += "        <lower> " + "  ".join([str(l) for l in doms[k,:,0]]) + " </lower>\n"
        xmlstr += "        <upper> " + "  ".join([str(u) for u in doms[k,:,1]]) + " </upper>\n"
        xmlstr += "      </bounds>\n"
        xmlstr += "      <sigma> \n"
        xmlstr += "        <lower> " + "  ".join([str(l) for l in lsigs[k]]) + " </lower>\n"
        xmlstr += "        <upper> " + "  ".join([str(u) for u in usigs[k]]) + " </upper>\n"
        xmlstr += "      </sigma>\n"
        xmlstr += "    </box>\n"
    xmlstr += "  </listboxes>\n"
    xmlstr += "</atlas>\n"
    return xmlstr

if __name__ == "__main__":
    xmlstr = genStringForFixedParams(test4DExample1,(8.0,0.6,0.6,0.6,12.0,2.5,2.5,2.5))
    # print(xmlstr)
    # variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds = xyz3DExample()
    # xmlstr = convertInputsToXML(variables, affectedby, maps, thresholds, loweramplitudes, upperamplitudes, lowerdecayrates, upperdecayrates, productionrates, upperbounds, lowerbounds)
    # print(xmlstr)
