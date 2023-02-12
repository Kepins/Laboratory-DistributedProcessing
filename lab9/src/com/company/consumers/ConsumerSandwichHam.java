package com.company.consumers;

import com.company.products.Bread;
import com.company.products.Butter;
import com.company.products.Ham;
import com.company.storages.Storage;

public class ConsumerSandwichHam implements Runnable{
    private Storage<Bread> breads;
    private Storage<Ham> hams;
    private Storage<Butter> butter;

    public ConsumerSandwichHam(Storage<Bread> breads, Storage<Ham> hams ,Storage<Butter> butter){
        this.breads = breads;
        this.hams = hams;
        this.butter = butter;
    }


    @Override
    public void run() {
        while (!Thread.interrupted()) {
            try {
                breads.take();
                hams.take();
                butter.take();
                System.out.println("Consumer made sandwich with ham");
            } catch (InterruptedException ex) {
                break;
            }
        }
    }
}
