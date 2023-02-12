package com.company.view;

import com.company.service.SessionService;

public class SessionStart {
    private final SessionService sessionService;

    public SessionStart(SessionService sessionService) {
        this.sessionService = sessionService;
    }


    public void start(int time) {
        System.out.println("Starting new session");
        try {
            Thread timeout = sessionService.startGenericSession();
            try {
                Thread.sleep(time);
            }catch(InterruptedException ex){

            }
            this.sessionService.getFinisher().setFinish();
            timeout.join();
            System.out.println("Session ends");
        } catch (InterruptedException ex) {

        }
    }
}
