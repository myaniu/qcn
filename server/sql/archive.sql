UPDATE continual.qcn_trigger SET flag=1 WHERE time_trigger < (unix_timestamp()-(3600*24*60));
INSERT INTO contarchive.qcn_trigger SELECT * FROM continual.qcn_trigger WHERE flag=1;
DELETE FROM continual.qcn_trigger WHERE flag=1;
OPTIMIZE TABLE contarchive.qcn_trigger;
OPTIMIZE TABLE continual.qcn_trigger;
UPDATE continual.result SET xml_doc_in=NULL, xml_doc_out=NULL where server_state>2;
UPDATE continual.workunit SET xml_doc=NULL,min_quorum=0,target_nresults=0 
   WHERE id IN (SELECT id FROM continual.result WHERE server_state>2);
OPTIMIZE TABLE continual.result;
OPTIMIZE TABLE continual.workunit;

UPDATE qcnalpha.qcn_trigger SET flag=1 WHERE time_trigger < (unix_timestamp()-(3600*24*60));
INSERT INTO qcnarchive.qcn_trigger SELECT * FROM qcnalpha.qcn_trigger WHERE flag=1;
DELETE FROM qcnalpha.qcn_trigger WHERE flag=1;
OPTIMIZE TABLE qcnarchive.qcn_trigger;
OPTIMIZE TABLE qcnalpha.qcn_trigger;
UPDATE qcnalpha.result SET xml_doc_in=NULL, xml_doc_out=NULL where server_state>2;
UPDATE qcnalpha.workunit SET xml_doc=NULL,min_quorum=0,target_nresults=0 
   WHERE id IN (SELECT id FROM qcnalpha.result WHERE server_state>2);
OPTIMIZE TABLE qcnalpha.result;
OPTIMIZE TABLE qcnalpha.workunit;
