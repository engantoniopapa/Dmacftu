# Dmacftu
Data Transfers between User and Kernel Memory via Dedicated DMA Engine

I made a kernel extension that adds to original kernel the functionalities and informations needed
to manage a DMA engine that performs memory-to-memory transfers. In particular, I chose to
use the vanilla kernel 3.9.2, while the hardware dedicated to the copies is a 5000 Series Chip-set
DMA Engine.
The extension that I developed not merely provides a basic interface for DMA transfers, but it
provides a set of policies used to manage the DMA channels. One of these policies can also be
used to manage the transfers of real-time applications, in fact it allows to assign exclusively the
bus of the DMA engine, for a certain time interval, to the channels that handle high-priority tasks.
Furthermore, to analyse the performance of a system that uses my extension I have developed a
tool-set that allowed to:  <br />
• study the behaviour of the memory copy functions used into the kernel;  <br />
• analyse the execution time of data transfers performed by the CPU and DMA;  <br />
• analyse the bandwidth of the system for data transfers executed by the CPU and DMA;  <br />
• study the phenomenon of Cache Pollution. <br />

