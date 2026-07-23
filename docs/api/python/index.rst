libCEC — Python API
===================

The ``cec`` module is the Python binding for libCEC, generated with SWIG from
the same C++ interface (``cec.h``) and protocol types (``cectypes.h``) as every
other binding. It exposes libCEC's ``ICECAdapter`` methods and the CEC enums and
structs to Python.

Getting started
---------------

.. code-block:: python

   import cec

   lib = cec.ICECAdapter.Create(cec.libcec_configuration())
   adapters = lib.DetectAdapters()
   if adapters:
       lib.Open(adapters[0].strComName)
       lib.PowerOnDevices(cec.CECDEVICE_TV)
       lib.Close()

See ``src/pyCecClient`` in the repository for a complete, runnable example
client.

API reference
-------------

.. automodule:: cec
   :members:
   :undoc-members:
   :show-inheritance:

Index
-----

* :ref:`genindex`
