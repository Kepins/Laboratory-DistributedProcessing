package com.company.timeout;

import com.company.finisher.Finisher;

import java.util.List;

public class Timeout implements Runnable{

    private List<Thread> threads;
    private Finisher finisher;

    public Timeout(List<Thread> threads, Finisher finisher) {
        this.threads = threads;
        this.finisher = finisher;
    }

    @Override
    public void run(){
        while(!finisher.shouldFinish()){
            try {
                Thread.sleep(250);
            } catch (InterruptedException ex) {

            }
        }
        for (Thread thread : threads) {
            thread.interrupt();
        }
        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException ex) {

            }
        }
    }

}
