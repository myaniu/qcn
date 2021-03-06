# This file was automatically generated by SWIG (http://www.swig.org).
# Version 2.0.9
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.



from sys import version_info
if version_info >= (2,6,0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_QCNTrigger', [dirname(__file__)])
        except ImportError:
            import _QCNTrigger
            return _QCNTrigger
        if fp is not None:
            try:
                _mod = imp.load_module('_QCNTrigger', fp, pathname, description)
            finally:
                fp.close()
            return _mod
    _QCNTrigger = swig_import_helper()
    del swig_import_helper
else:
    import _QCNTrigger
del version_info
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError(name)

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


class doubleArray(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, doubleArray, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, doubleArray, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _QCNTrigger.new_doubleArray(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _QCNTrigger.delete_doubleArray
    __del__ = lambda self : None;
    def __getitem__(self, *args): return _QCNTrigger.doubleArray___getitem__(self, *args)
    def __setitem__(self, *args): return _QCNTrigger.doubleArray___setitem__(self, *args)
    def cast(self): return _QCNTrigger.doubleArray_cast(self)
    __swig_getmethods__["frompointer"] = lambda x: _QCNTrigger.doubleArray_frompointer
    if _newclass:frompointer = staticmethod(_QCNTrigger.doubleArray_frompointer)
doubleArray_swigregister = _QCNTrigger.doubleArray_swigregister
doubleArray_swigregister(doubleArray)

def doubleArray_frompointer(*args):
  return _QCNTrigger.doubleArray_frompointer(*args)
doubleArray_frompointer = _QCNTrigger.doubleArray_frompointer

class floatArray(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, floatArray, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, floatArray, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _QCNTrigger.new_floatArray(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _QCNTrigger.delete_floatArray
    __del__ = lambda self : None;
    def __getitem__(self, *args): return _QCNTrigger.floatArray___getitem__(self, *args)
    def __setitem__(self, *args): return _QCNTrigger.floatArray___setitem__(self, *args)
    def cast(self): return _QCNTrigger.floatArray_cast(self)
    __swig_getmethods__["frompointer"] = lambda x: _QCNTrigger.floatArray_frompointer
    if _newclass:frompointer = staticmethod(_QCNTrigger.floatArray_frompointer)
floatArray_swigregister = _QCNTrigger.floatArray_swigregister
floatArray_swigregister(floatArray)

def floatArray_frompointer(*args):
  return _QCNTrigger.floatArray_frompointer(*args)
floatArray_frompointer = _QCNTrigger.floatArray_frompointer

class intArray(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, intArray, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, intArray, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _QCNTrigger.new_intArray(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _QCNTrigger.delete_intArray
    __del__ = lambda self : None;
    def __getitem__(self, *args): return _QCNTrigger.intArray___getitem__(self, *args)
    def __setitem__(self, *args): return _QCNTrigger.intArray___setitem__(self, *args)
    def cast(self): return _QCNTrigger.intArray_cast(self)
    __swig_getmethods__["frompointer"] = lambda x: _QCNTrigger.intArray_frompointer
    if _newclass:frompointer = staticmethod(_QCNTrigger.intArray_frompointer)
intArray_swigregister = _QCNTrigger.intArray_swigregister
intArray_swigregister(intArray)

def intArray_frompointer(*args):
  return _QCNTrigger.intArray_frompointer(*args)
intArray_frompointer = _QCNTrigger.intArray_frompointer

class QCNTrigger(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, QCNTrigger, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, QCNTrigger, name)
    __repr__ = _swig_repr
    P = _QCNTrigger.QCNTrigger_P
    S = _QCNTrigger.QCNTrigger_S
    def __init__(self): 
        this = _QCNTrigger.new_QCNTrigger()
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _QCNTrigger.delete_QCNTrigger
    __del__ = lambda self : None;
    def copyFromUSBTrigger(self, *args): return _QCNTrigger.QCNTrigger_copyFromUSBTrigger(self, *args)
    def _print(self): return _QCNTrigger.QCNTrigger__print(self)
    def setMagnitude(self): return _QCNTrigger.QCNTrigger_setMagnitude(self)
    N_SHORT = _QCNTrigger.QCNTrigger_N_SHORT
    __swig_setmethods__["qcn_quakeid"] = _QCNTrigger.QCNTrigger_qcn_quakeid_set
    __swig_getmethods__["qcn_quakeid"] = _QCNTrigger.QCNTrigger_qcn_quakeid_get
    if _newclass:qcn_quakeid = _swig_property(_QCNTrigger.QCNTrigger_qcn_quakeid_get, _QCNTrigger.QCNTrigger_qcn_quakeid_set)
    __swig_setmethods__["posted"] = _QCNTrigger.QCNTrigger_posted_set
    __swig_getmethods__["posted"] = _QCNTrigger.QCNTrigger_posted_get
    if _newclass:posted = _swig_property(_QCNTrigger.QCNTrigger_posted_get, _QCNTrigger.QCNTrigger_posted_set)
    __swig_setmethods__["hostid"] = _QCNTrigger.QCNTrigger_hostid_set
    __swig_getmethods__["hostid"] = _QCNTrigger.QCNTrigger_hostid_get
    if _newclass:hostid = _swig_property(_QCNTrigger.QCNTrigger_hostid_get, _QCNTrigger.QCNTrigger_hostid_set)
    __swig_setmethods__["triggerid"] = _QCNTrigger.QCNTrigger_triggerid_set
    __swig_getmethods__["triggerid"] = _QCNTrigger.QCNTrigger_triggerid_get
    if _newclass:triggerid = _swig_property(_QCNTrigger.QCNTrigger_triggerid_get, _QCNTrigger.QCNTrigger_triggerid_set)
    __swig_setmethods__["db"] = _QCNTrigger.QCNTrigger_db_set
    __swig_getmethods__["db"] = _QCNTrigger.QCNTrigger_db_get
    if _newclass:db = _swig_property(_QCNTrigger.QCNTrigger_db_get, _QCNTrigger.QCNTrigger_db_set)
    __swig_setmethods__["file"] = _QCNTrigger.QCNTrigger_file_set
    __swig_getmethods__["file"] = _QCNTrigger.QCNTrigger_file_get
    if _newclass:file = _swig_property(_QCNTrigger.QCNTrigger_file_get, _QCNTrigger.QCNTrigger_file_set)
    __swig_setmethods__["result_name"] = _QCNTrigger.QCNTrigger_result_name_set
    __swig_getmethods__["result_name"] = _QCNTrigger.QCNTrigger_result_name_get
    if _newclass:result_name = _swig_property(_QCNTrigger.QCNTrigger_result_name_get, _QCNTrigger.QCNTrigger_result_name_set)
    __swig_setmethods__["longitude"] = _QCNTrigger.QCNTrigger_longitude_set
    __swig_getmethods__["longitude"] = _QCNTrigger.QCNTrigger_longitude_get
    if _newclass:longitude = _swig_property(_QCNTrigger.QCNTrigger_longitude_get, _QCNTrigger.QCNTrigger_longitude_set)
    __swig_setmethods__["latitude"] = _QCNTrigger.QCNTrigger_latitude_set
    __swig_getmethods__["latitude"] = _QCNTrigger.QCNTrigger_latitude_get
    if _newclass:latitude = _swig_property(_QCNTrigger.QCNTrigger_latitude_get, _QCNTrigger.QCNTrigger_latitude_set)
    __swig_setmethods__["time_trigger"] = _QCNTrigger.QCNTrigger_time_trigger_set
    __swig_getmethods__["time_trigger"] = _QCNTrigger.QCNTrigger_time_trigger_get
    if _newclass:time_trigger = _swig_property(_QCNTrigger.QCNTrigger_time_trigger_get, _QCNTrigger.QCNTrigger_time_trigger_set)
    __swig_setmethods__["time_received"] = _QCNTrigger.QCNTrigger_time_received_set
    __swig_getmethods__["time_received"] = _QCNTrigger.QCNTrigger_time_received_get
    if _newclass:time_received = _swig_property(_QCNTrigger.QCNTrigger_time_received_get, _QCNTrigger.QCNTrigger_time_received_set)
    __swig_setmethods__["time_est"] = _QCNTrigger.QCNTrigger_time_est_set
    __swig_getmethods__["time_est"] = _QCNTrigger.QCNTrigger_time_est_get
    if _newclass:time_est = _swig_property(_QCNTrigger.QCNTrigger_time_est_get, _QCNTrigger.QCNTrigger_time_est_set)
    __swig_setmethods__["significance"] = _QCNTrigger.QCNTrigger_significance_set
    __swig_getmethods__["significance"] = _QCNTrigger.QCNTrigger_significance_get
    if _newclass:significance = _swig_property(_QCNTrigger.QCNTrigger_significance_get, _QCNTrigger.QCNTrigger_significance_set)
    __swig_setmethods__["magnitude"] = _QCNTrigger.QCNTrigger_magnitude_set
    __swig_getmethods__["magnitude"] = _QCNTrigger.QCNTrigger_magnitude_get
    if _newclass:magnitude = _swig_property(_QCNTrigger.QCNTrigger_magnitude_get, _QCNTrigger.QCNTrigger_magnitude_set)
    __swig_setmethods__["pgah"] = _QCNTrigger.QCNTrigger_pgah_set
    __swig_getmethods__["pgah"] = _QCNTrigger.QCNTrigger_pgah_get
    if _newclass:pgah = _swig_property(_QCNTrigger.QCNTrigger_pgah_get, _QCNTrigger.QCNTrigger_pgah_set)
    __swig_setmethods__["pgaz"] = _QCNTrigger.QCNTrigger_pgaz_set
    __swig_getmethods__["pgaz"] = _QCNTrigger.QCNTrigger_pgaz_get
    if _newclass:pgaz = _swig_property(_QCNTrigger.QCNTrigger_pgaz_get, _QCNTrigger.QCNTrigger_pgaz_set)
    __swig_setmethods__["c_cnt"] = _QCNTrigger.QCNTrigger_c_cnt_set
    __swig_getmethods__["c_cnt"] = _QCNTrigger.QCNTrigger_c_cnt_get
    if _newclass:c_cnt = _swig_property(_QCNTrigger.QCNTrigger_c_cnt_get, _QCNTrigger.QCNTrigger_c_cnt_set)
    __swig_setmethods__["c_ind"] = _QCNTrigger.QCNTrigger_c_ind_set
    __swig_getmethods__["c_ind"] = _QCNTrigger.QCNTrigger_c_ind_get
    if _newclass:c_ind = _swig_property(_QCNTrigger.QCNTrigger_c_ind_get, _QCNTrigger.QCNTrigger_c_ind_set)
    __swig_setmethods__["c_hid"] = _QCNTrigger.QCNTrigger_c_hid_set
    __swig_getmethods__["c_hid"] = _QCNTrigger.QCNTrigger_c_hid_get
    if _newclass:c_hid = _swig_property(_QCNTrigger.QCNTrigger_c_hid_get, _QCNTrigger.QCNTrigger_c_hid_set)
    __swig_setmethods__["dis"] = _QCNTrigger.QCNTrigger_dis_set
    __swig_getmethods__["dis"] = _QCNTrigger.QCNTrigger_dis_get
    if _newclass:dis = _swig_property(_QCNTrigger.QCNTrigger_dis_get, _QCNTrigger.QCNTrigger_dis_set)
    __swig_setmethods__["pors"] = _QCNTrigger.QCNTrigger_pors_set
    __swig_getmethods__["pors"] = _QCNTrigger.QCNTrigger_pors_get
    if _newclass:pors = _swig_property(_QCNTrigger.QCNTrigger_pors_get, _QCNTrigger.QCNTrigger_pors_set)
    __swig_setmethods__["dirty"] = _QCNTrigger.QCNTrigger_dirty_set
    __swig_getmethods__["dirty"] = _QCNTrigger.QCNTrigger_dirty_get
    if _newclass:dirty = _swig_property(_QCNTrigger.QCNTrigger_dirty_get, _QCNTrigger.QCNTrigger_dirty_set)
QCNTrigger_swigregister = _QCNTrigger.QCNTrigger_swigregister
QCNTrigger_swigregister(QCNTrigger)

# This file is compatible with both classic and new-style classes.


