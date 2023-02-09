package com.company;

import com.company.finisher.Finisher;
import com.company.service.SessionParameters;
import com.company.service.SessionService;
import com.company.view.SessionStart;

public class Application {

    public static void main(String[] args) {
        Finisher finisher = new Finisher();
        SessionParameters sessionParameters = new SessionParameters(1, 1,
                1, 30,30, 25);
        SessionService sessionService = new SessionService(sessionParameters, finisher);
        SessionStart sessionStart = new SessionStart(sessionService);

        sessionStart.start(10000);
    }
}
