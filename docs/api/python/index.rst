libCEC — Python API
===================

The ``cec`` module is the Python binding for libCEC, generated with SWIG from the
same C++ interface (``cec.h``) and protocol types (``cectypes.h``) as every other
binding. It lets Python applications control CEC-capable HDMI devices — power a TV
on or to standby, become the active source, send remote keys, read device state —
and receive bus events.

Every binding wraps the same engine, so the concepts (logical and physical
addresses, device types, opcodes, power states) match the
`C/C++ reference <https://pulse-eight.github.io/libcec/cpp/>`_, which documents
them in the most depth.

Requirements
------------

- A **Pulse-Eight USB-CEC adapter** (or a supported SoC-native CEC backend).
- The native **libCEC** library and its Python module (``cec``).
- **Python 3**.

Installing
----------

Debian / Ubuntu
~~~~~~~~~~~~~~~~

.. code-block:: sh

   sudo apt-get install python3-libcec

This installs the ``cec`` module against the system libCEC.

Build from source
~~~~~~~~~~~~~~~~~~

The Python binding is built automatically when SWIG and the Python development
files are present:

.. code-block:: sh

   sudo apt-get install swig python3-dev      # build prerequisites
   git clone https://github.com/Pulse-Eight/libcec.git
   mkdir libcec/build && cd libcec/build
   cmake ..
   make -j4
   sudo make install && sudo ldconfig

``cmake`` reports ``Python support: yes`` when it detects the prerequisites.

Usage
-----

Create a :class:`cec.libcec_configuration`, optionally attach callbacks, create
the adapter with :meth:`cec.ICECAdapter.Create`, then open a connection.

.. code-block:: python

   import cec

   # 1. describe this client
   config = cec.libcec_configuration()
   config.strDeviceName   = "pyCec"
   config.bActivateSource = 0
   config.deviceTypes.Add(cec.CEC_DEVICE_TYPE_RECORDING_DEVICE)
   config.clientVersion   = cec.LIBCEC_VERSION_CURRENT

   # 2. (optional) receive events — callbacks are set on the configuration
   def log_callback(level, time, message):
       print("log:", message)
       return 0

   config.SetLogCallback(log_callback)

   # 3. create the instance and open the first adapter found
   lib = cec.ICECAdapter.Create(config)
   adapters = lib.DetectAdapters()
   if adapters:
       lib.Open(adapters[0].strComName)

       # 4. control the bus
       lib.PowerOnDevices(cec.CECDEVICE_TV)
       print("TV:", lib.GetDeviceOSDName(cec.CECDEVICE_TV))

       # 5. clean up
       lib.Close()

Callbacks (``SetLogCallback``, ``SetKeyPressCallback``, ``SetCommandCallback``)
are invoked from libCEC's worker thread. See ``src/pyCecClient/pyCecClient.py`` in
the repository for a complete, runnable example client.

API reference
-------------

.. automodule:: cec
   :members:
   :undoc-members:
   :show-inheritance:

Index
-----

* :ref:`genindex`
