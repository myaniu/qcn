/* CMC -- this is the old do_stats python moved to a stored procedure
   to hopefully run faster!

   One "wall-clock day" of QCN is worth 50 cobblestones per Dave Anderson suggestion

   also check msg_from_host table for variety "finalstats" and handled=0 for this xml:

*/

DELIMITER //

DROP PROCEDURE IF EXISTS do_stats
//

CREATE PROCEDURE do_stats()
BEGIN

    SET AUTOCOMMIT=1;

    /* first off run the do_final_trigger() stored procedure to process any finished workunit trigger stats */
    CALL do_final_trigger();

    /* make a subset of result table */
    TRUNCATE TABLE qcn_recalcresult;

    /* first insert the final result records (final stats trickles) */
    INSERT INTO qcn_recalcresult
      (SELECT resultid,
       exp(-(abs(unix_timestamp()-time_received))*0.69314718/604800.0) weight,
         (50.0*runtime_clock)/86400.0 total_credit
         FROM qcn_finalstats);

    /* note the left join below to ignore triggers for resultid's that we received a final stats trickle for above */
    INSERT INTO qcn_recalcresult
      (SELECT r.id resultid,                              
       exp(-(abs(unix_timestamp()-max(t.time_received))*0.69314718/604800.0)) weight,
         (50.0*max(t.runtime_clock)/86400.0) total_credit
         FROM result r
           LEFT JOIN qcn_finalstats f ON r.id=f.resultid 
           JOIN qcn_trigger t ON r.name=t.result_name
         WHERE f.resultid IS NULL and t.runtime_clock>0
                GROUP BY r.id);

    TRUNCATE TABLE qcn_stats;
    INSERT INTO qcn_stats 
       SELECT r.userid,r.hostid,u.teamid,r.id,c.total_credit,c.weight
        FROM result r, user u, 
         qcn_recalcresult c 
       WHERE r.id=c.resultid AND r.userid=u.id;

    UPDATE result u SET granted_credit=
      ROUND(IFNULL((SELECT total_credit FROM qcn_recalcresult r WHERE r.resultid=u.id),0),3), claimed_credit=granted_credit;

    UPDATE user u SET total_credit=IFNULL((select sum(total_credit) from 
          qcn_stats r WHERE r.userid=u.id),0),
             expavg_credit=IFNULL((SELECT SUM(weight*total_credit) FROM 
               qcn_stats rs WHERE rs.userid=u.id),0),
             expavg_time=unix_timestamp();

    UPDATE host u SET total_credit=IFNULL((select sum(total_credit) from 
          qcn_stats r WHERE r.hostid=u.id),0),
             expavg_credit=IFNULL((SELECT SUM(weight*total_credit) FROM 
               qcn_stats rs WHERE rs.hostid=u.id),0),
             expavg_time=unix_timestamp();

    UPDATE team u SET total_credit=IFNULL((select sum(total_credit) from 
          qcn_stats r WHERE r.teamid=u.id),0),
             expavg_credit=IFNULL((SELECT SUM(weight*total_credit) FROM 
               qcn_stats rs WHERE rs.teamid=u.id),0),
             expavg_time=unix_timestamp();

    COMMIT;
END
//

DELIMITER ;

