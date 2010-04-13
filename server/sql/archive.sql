UPDATE qcnalpha.qcn_trigger SET flag=1 WHERE time_trigger < (unix_timestamp()-(3600*24*60));
INSERT INTO qcnarchive.qcn_trigger SELECT * FROM qcnalpha.qcn_trigger WHERE flag=1;
DELETE FROM qcnalpha.qcn_trigger WHERE flag=1;
OPTIMIZE TABLE qcnarchive.qcn_trigger;
OPTIMIZE TABLE qcnalpha.qcn_trigger;
